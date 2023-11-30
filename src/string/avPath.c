#include <AvUtils/string/avPath.h>
#include <AvUtils/string/avChar.h>
#include <AvUtils/avMemory.h>
#include <AvUtils/avLogging.h>
#define AV_DYNAMIC_ARRAY_EXPOSE_MEMORY_LAYOUT
#include <AvUtils/dataStructures/avDynamicArray.h>
#include <string.h>
#include <stdio.h>

#ifdef _WIN32
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif

typedef struct  Directory {
    AvStringMemory memory;
    AvString str;
    struct Directory* child;
    struct Directory* parent;
} Directory;

typedef struct AvPath_T {
    Directory* root;
    bool32 absolute;
    AvString cachedString;
} AvPath_T;

static void getCwd(AvStringRef str);

static Directory* createPathList(AvString pathStr, Directory** root) {
    AvArray directories = AV_EMPTY;
    AvString pathStrTmp = AV_EMPTY;
    avStringClone(&pathStrTmp, pathStr);

    if (avStringEndsWithChar(pathStrTmp, '/')) {
        avStringWrite(&pathStrTmp, avStringFindLastOccuranceOfChar(pathStrTmp, '/'), '\0');
        avStringMemoryResize(pathStrTmp.len - 1, pathStrTmp.memory);
    }

    avStringSplitOnChar(&directories, '/', pathStrTmp);
    avStringFree(&pathStrTmp);
    Directory* prevDirPtr = nullptr;
    avArrayForEachElement(AvString, element, index, &directories, {
        Directory * dirPtr = avCallocate(1, sizeof(Directory), "allocating directory path");
        dirPtr->parent = prevDirPtr;
        if (prevDirPtr) {
            prevDirPtr->child = dirPtr;
        } else {
            (*root) = dirPtr;
        }
        prevDirPtr = dirPtr;
        if (element.len != 0) {
            avStringMemoryAllocStore(element, &dirPtr->memory);
            avStringFromMemory(&dirPtr->str,0, AV_STRING_FULL_LENGTH, &dirPtr->memory);
        }
        });
    avArrayFree(&directories);
    return prevDirPtr;
}

static void destroyPathList(Directory* dir) {
    if (!dir) {
        return;
    }
    destroyPathList(dir->child);
    avStringFree(&dir->str);
    avStringMemoryFree(&dir->memory);
}

void avPathCreate(AvString pathStr, AvPath* path) {
    (*path) = avCallocate(1, sizeof(AvPath_T), "allocating path handle");
    createPathList(pathStr, &(*path)->root);
}

static bool32 stringIsWinDriveLetter(AvString str) {
    if (str.len != 2) {
        return false;
    }
    if (avCharIsLetter(str.chrs[0]) && str.chrs[1] == ':') {
        return true;
    }
    return false;
}

static bool32 isRoot(AvPath path) {
    avAssert(path != nullptr, "path must be valid");
    avAssert(path->root != nullptr, "path must have a path");

    if (path->root->str.memory == nullptr) {
        return true;
    }

#ifdef _WIN32
    if (stringIsWinDriveLetter(path->root->str)) {
        return true;
    }
#endif
    return false;
}

static void removeDir(Directory* dir, AvPath path) {
    if (dir->parent) {
        dir->parent->child = dir->child;
    }
    if (dir->child) {
        dir->child->parent = dir->parent;
    }
    if (dir == path->root) {
        path->root = dir->child;
    }
    dir->child = nullptr;
    destroyPathList(dir);

}

static void cachedStringInvalidate(AvPath path) {
    avStringFree(&path->cachedString);
}

void avPathSimplify(AvPath path) {
    avAssert(path != nullptr, "path must be valid");

    Directory* dir = path->root;
    if (isRoot(path)) {
        if (!dir->child) {
            return;
        }
        dir = dir->child;
    }
    while (dir) {
        Directory* next = dir->child;
        if (avStringEquals(dir->str, AV_CSTR("."))) {
            removeDir(dir, path);
            dir = next;
            continue;
        }
        if (avStringEquals(dir->str, AV_CSTR(".."))) {
            if (dir->parent && !(dir->parent->str.memory == nullptr
#ifdef _WIN32
                || stringIsWinDriveLetter(dir->parent->str)
#endif
                )) {
                removeDir(dir->parent, path);
                removeDir(dir, path);
            }
        }

        dir = next;
    }
    cachedStringInvalidate(path);
}

void avPathMakeAbsolute(AvPath path) {
    avAssert(path != nullptr, "path must be a valid handle");
    avAssert(path->root != nullptr, "path must not be empty");
    if (isRoot(path)) {
        return;
    }

    AvString cwd = AV_EMPTY;
    Directory* root;
    getCwd(&cwd);
    Directory* lastCwdDir = createPathList(cwd, &root);
    avStringFree(&cwd);
    if (path->root->str.memory == nullptr) {
        if (path->root->child == nullptr) {
            path->root = root;
            cachedStringInvalidate(path);
            return;
        }
        path->root->child->parent = lastCwdDir;
        lastCwdDir->child = path->root->child;
    } else {
        path->root->parent = lastCwdDir;
        lastCwdDir->child = path->root;
    }
    path->root = root;
    cachedStringInvalidate(path);
}

void avPathGetStr(AvStringRef str, AvPath path) {
    avAssert(str != nullptr, "string must be a valid reference");
    avAssert(path != nullptr, "path must be a valid reference");

    if (path->cachedString.memory) {
        avStringClone(str, path->cachedString);
        return;
    }

    uint64 length = 0;
    Directory* dir = path->root;
    while (dir) {
        length += dir->str.len + 1;
        dir = dir->child;
    }
    length-=1;
    AvStringHeapMemory mem;
    avStringMemoryHeapAllocate(length, &mem);
    uint32 writeIndex = 0;
    dir = path->root;
    while (dir) {
        avStringMemoryStore(dir->str, writeIndex, dir->str.len, mem);
        writeIndex += dir->str.len;
        if (dir->child) {
            avStringMemoryStore(AV_STR("/", 1), writeIndex, 1, mem);
            writeIndex += 1;
        }
        dir = dir->child;
    }
    avStringFromMemory(str, 0, AV_STRING_FULL_LENGTH, mem);
    avStringClone(&path->cachedString, *str);
}

void avPathChangeDirectory(AvString str, AvPath path) {
    Directory* dir;
    createPathList(str, &dir);
    if (dir->str.memory == nullptr
#ifdef _WIN32
        || stringIsWinDriveLetter(dir->str)
#endif
        ) {
        destroyPathList(path->root);
        path->root = dir;
    } else {
        Directory* lastDir = path->root;
        while (true) {
            if (lastDir->child) {
                lastDir = lastDir->child;
                continue;
            }
            break;
        }
        lastDir->child = dir;
        dir->parent = lastDir;
    }

    cachedStringInvalidate(path);
}

void avPathClone(AvPath* dst, AvPath src) {
    AvString str = AV_EMPTY;
    avPathGetStr(&str, src);
    avPathCreate(str, dst);
    avStringFree(&str);
}

void avPathDestroy(AvPath path) {
    avStringFree(&path->cachedString);
    destroyPathList(path->root);
    avFree(path);
}


static void getCwd(AvStringRef str) {
    char buffer[FILENAME_MAX + 1];
    GetCurrentDir(
        buffer,
        FILENAME_MAX
    );
    avStringClone(str, AV_CSTR(buffer));
    avStringReplaceChar(str, '\\', '/');
    if (avStringEndsWithChar(*str, '/')) {
        avStringWrite(str, str->len - 1, '\0');
        avStringMemoryResize(str->len - 1, str->memory);
    }
}