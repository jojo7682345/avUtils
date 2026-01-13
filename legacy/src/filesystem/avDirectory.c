#include <AvUtils/filesystem/avDirectory.h>
#include <AvUtils/avMemory.h>
#define AV_DYNAMIC_ARRAY_EXPOSE_MEMORY_LAYOUT
#include <AvUtils/dataStructures/avDynamicArray.h>
#include <AvUtils/dataStructures/avArray.h>
#include <AvUtils/avLogging.h>
#include <AvUtils/string/avChar.h>

#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>

#include <shlobj.h>     // SHFileOperation
#include <strsafe.h>

int mkdir(const char* dir, unsigned int mode ){
    return CreateDirectory(dir, NULL);
}

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#ifndef S_IRWXU
#define S_IRWXU (0400|0200|0100)
#endif

#ifndef S_IRWXG 
#define S_IRWXG ((0400|0200|0100) >> 3)
#endif

#ifndef S_IROTH 
#define S_IROTH ((0400 >> 3) >> 3)
#endif

#ifndef S_IXOTH
#define S_IXOTH ((0100 >> 3) >> 3)
#endif

#else
#include <unistd.h>
#ifndef __USE_MISC
#define __USE_MISC
#endif
#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>
#include <linux/limits.h>
#endif

static int mkdirs(const char *dir, unsigned int mode) {
    char tmp[PATH_MAX];
    char *p = NULL;
    size_t len;

    snprintf(tmp, sizeof(tmp),"%s",dir);
    len = strlen(tmp);
    if (tmp[len - 1] == '/'){
        tmp[len - 1] = 0;
    }
    for (p = tmp + 1; *p; p++){
        if (*p == '/') {
            *p = 0;
            mkdir(tmp, mode);
            *p = '/';
        }
    }
    return mkdir(tmp, mode);
}


bool32 avGetCurrentDir(uint64 bufferSize, char* buffer){
#ifndef _WIN32
    return (getcwd(buffer,bufferSize) != NULL);
#else
    return (GetCurrentDirectory(bufferSize, buffer) != 0);
#endif
}

int32 avChangeCurrentDir(AvString dir){
    AvString tmpStr = AV_EMPTY;
    avStringClone(&tmpStr, dir);

    #ifndef _WIN32
        int ret = chdir(tmpStr.chrs);
    #else
        int ret =  SetCurrentDirectory(tmpStr.chrs) ? 0 : -1;
    #endif

    avStringFree(&tmpStr);
    return ret;
}

static AvPathNodeType pathGetType(AvString str) {
    AvString path = AV_EMPTY;
    avStringClone(&path, str);

    AvPathNodeType type = AV_PATH_NODE_TYPE_NONE;
#ifndef _WIN32
    struct stat buffer;
    int result = stat(path.chrs, &buffer);

    if (result != 0) {
        type = AV_PATH_NODE_TYPE_NONE;
        goto typeFound;
    }
    if ((buffer.st_mode & S_IFDIR) != 0) {
        type = AV_PATH_NODE_TYPE_DIRECTORY;
        goto typeFound;
    }
    type = AV_PATH_NODE_TYPE_FILE;
#else
    DWORD attributes = GetFileAttributesA(path.chrs);
    if (attributes == INVALID_FILE_ATTRIBUTES) {
        type = AV_PATH_NODE_TYPE_NONE;
        goto typeFound;
    }

    if ((attributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
        type = AV_PATH_NODE_TYPE_DIRECTORY;
        goto typeFound;
    }

    type = AV_PATH_NODE_TYPE_FILE;
#endif
typeFound:
    avStringFree(&path);
    return type;
}



bool32 avDirectoryExists(AvString location){
    AvString tmpStr = AV_EMPTY;
    avStringClone(&tmpStr, location);
    bool32 ret = false;
#ifndef _WIN32
    DIR* dir = opendir(tmpStr.chrs);
    if (dir) {
        ret = true;
        closedir(dir);
    } else if (ENOENT == errno) {
        ret = false;
    } else {
        avAssert(false, "opendir failed");
    }
#else
    DWORD attributes = GetFileAttributesA(tmpStr.chrs);
    ret = (attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY));
#endif
    avStringFree(&tmpStr);
    return ret;
}

uint32 avMakeDirectory(AvString location){
    AvString tmpStr = AV_EMPTY;
    avStringClone(&tmpStr, location);
    int retCode = mkdir(tmpStr.chrs, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    avStringFree(&tmpStr);
    return retCode;
} 

uint32 avMakeDirectoryRecursive(AvString location){
    AvString tmpStr = AV_EMPTY;
    avStringClone(&tmpStr, location);
    int retCode = mkdirs(tmpStr.chrs, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    avStringFree(&tmpStr);
    return retCode;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wjump-misses-init"
bool32 avDirectoryOpen(AvString location, AvPath* root, AvPathRef pathRef){
    avStringDebugContextStart;
    AvPath path = {0};
    AvString fullPath = AV_EMPTY;
    if(root){
        path.root = root;
        path.allocator = root->allocator;
        avStringJoin(&fullPath, root->path, AV_CSTR("/"), location);
    }else{
        path.root = nullptr;
        avAllocatorCreate(0, AV_ALLOCATOR_TYPE_DYNAMIC, &path.allocator);
        avStringClone(&fullPath, location);
    }
    avStringReplaceChar(&fullPath, '\\', '/');
    AvString fullPathFixed = {
        .chrs = fullPath.chrs,
        .len = fullPath.len - (avStringEndsWith(fullPath, AV_CSTRA("/")) ? 1 : 0),
        .memory = fullPath.memory,
    };
    avStringUnsafeCopy(&fullPath, fullPathFixed);
    avStringCopyToAllocator(fullPath, &path.path, &path.allocator);
    if(pathGetType(fullPath)!=AV_PATH_NODE_TYPE_DIRECTORY){
        goto pathNotDirectory;
    }

    AvDynamicArray entries = nullptr;
    avDynamicArrayCreate(0, sizeof(AvPathNode), &entries);

#ifndef _WIN32

    DIR* dir = opendir(fullPath.chrs);
    if(!dir){
        goto pathNotFound;
    }

    struct dirent* entry = readdir(dir);
    while(entry){
        if(strcmp(entry->d_name,".")==0 || strcmp(entry->d_name,"..")==0){
            goto next;
        }

        AvString entryName = {
            .chrs = entry->d_name,
            .len = avCStringLength(entry->d_name),
            .memory = nullptr,
        };
        AvString entryPath = {0};
        avStringJoin(&entryPath, fullPath, AV_CSTR("/"), entryName);
        struct stat stats = {0};
        stat(entryPath.chrs, &stats);
        AvPathNodeType type = AV_PATH_NODE_TYPE_FILE;
        if((stats.st_mode & S_IFDIR) != 0){
            type = AV_PATH_NODE_TYPE_DIRECTORY;
        }
        AvPathNode node = {
            .name = AV_EMPTY,
            .type = type,
        };

        avStringCopyToAllocator(entryPath, &node.fullName, &path.allocator);
        AvString fileName = {
            .chrs = node.fullName.chrs + fullPath.len + 1,
            .len = node.fullName.len - fullPath.len - 1,
            .memory = nullptr,
        };
        memcpy(&node.name, &fileName, sizeof(AvString));
        avStringFree(&entryPath);
        avDynamicArrayAdd(&node, entries);
    next:
        entry = readdir(dir);
    }

    closedir(dir);

#else

    WIN32_FIND_DATA findFileData;
    HANDLE hFind = INVALID_HANDLE_VALUE;

    AvString searchPath = {0};
    avStringJoin(&searchPath, fullPath, AV_CSTR("\\*")); // Add wildcard to search directory contents

    hFind = FindFirstFile(searchPath.chrs, &findFileData);
    if (hFind == INVALID_HANDLE_VALUE) {
        avStringFree(&searchPath);
        goto pathNotFound;
    }

    do {
        // Skip the "." and ".." entries
        if (strcmp(findFileData.cFileName, ".") == 0 || strcmp(findFileData.cFileName, "..") == 0) {
            continue;
        }

        AvString entryName = {
            .chrs = findFileData.cFileName,
            .len = avCStringLength(findFileData.cFileName),
            .memory = nullptr,
        };

        AvString entryPath = {0};
        avStringJoin(&entryPath, fullPath, AV_CSTR("/"), entryName);

        AvPathNodeType type = AV_PATH_NODE_TYPE_FILE;
        if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            type = AV_PATH_NODE_TYPE_DIRECTORY;
        }

        AvPathNode node = {
            .name = AV_EMPTY,
            .type = type,
        };

        avStringCopyToAllocator(entryPath, &node.fullName, &path.allocator);
        AvString fileName = {
            .chrs = node.fullName.chrs + fullPath.len + 1,
            .len = node.fullName.len - fullPath.len - 1,
            .memory = nullptr,
        };
        memcpy(&node.name, &fileName, sizeof(AvString));
        avStringFree(&entryPath);
        avDynamicArrayAdd(&node, entries);

    } while (FindNextFile(hFind, &findFileData) != 0);

    FindClose(hFind);
    avStringFree(&searchPath);

#endif

    path.contentCount = avDynamicArrayGetSize(entries);
    if(path.contentCount){
        AvPathNode* content = avAllocatorAllocate(sizeof(AvPathNode)*path.contentCount, &path.allocator);
        avDynamicArrayReadRange(content, path.contentCount, 0, sizeof(AvPathNode), 0, entries);
        path.content = content;
    }
    avDynamicArrayDestroy(entries);
    
    avStringFree(&fullPath);
    avStringDebugContextEnd;

    memcpy(pathRef, &path, sizeof(AvPath));
    return true;

pathNotFound:
    avDynamicArrayDestroy(entries);
pathNotDirectory:
    if(!root){
        avAllocatorDestroy(&path.allocator);
    }

    avStringFree(&fullPath);
    avStringDebugContextEnd;

    return false;
}
#pragma GCC diagnostic pop

void avDirectoryClose(AvPathRef path){
    if(path->root){
        return;
    }
    avAllocatorDestroy(&path->allocator);
}

#ifndef _WIN32
static bool32 avDirectoryDeleteRecursivePosix(const char* path) {
    DIR* dir = opendir(path);
    if (!dir)
        return rmdir(path) == 0;

    struct dirent* entry;
    while ((entry = readdir(dir))) {
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
            continue;

        char fullPath[4096];
        snprintf(fullPath, sizeof(fullPath), "%s/%s", path, entry->d_name);

        struct stat st;
        if (stat(fullPath, &st) != 0)
            continue;

        if (S_ISDIR(st.st_mode)) {
            if (!avDirectoryDeleteRecursivePosix(fullPath)) {
                closedir(dir);
                return 0;
            }
        } else {
            if (unlink(fullPath) != 0) {
                closedir(dir);
                return 0;
            }
        }
    }

    closedir(dir);
    return rmdir(path) == 0;
}
#endif
#ifdef _WIN32
static bool32 avDirectoryDeleteRecursiveWin32(const char* path) {
    DWORD attrib = GetFileAttributesA(path);
    if (attrib == INVALID_FILE_ATTRIBUTES ||
        !(attrib & FILE_ATTRIBUTE_DIRECTORY)) {
        return 0;
    }

    TCHAR szDir[MAX_PATH];
    StringCchCopyA(szDir, MAX_PATH, path);
    StringCchCatA(szDir, MAX_PATH, "\\*");

    int len = lstrlen(szDir);
    TCHAR* from = (TCHAR*)malloc((len + 2) * sizeof(TCHAR));
    if (!from)
        return 0;

    StringCchCopyA(from, len + 2, szDir);
    from[len]     = '\0';
    from[len + 1] = '\0'; // double-null termination

    SHFILEOPSTRUCTA fileop = {0};
    fileop.wFunc  = FO_DELETE;
    fileop.pFrom  = from;
    fileop.fFlags = FOF_NOCONFIRMATION |
                    FOF_NOERRORUI |
                    FOF_SILENT;

    int result = SHFileOperationA(&fileop);
    free(from);

    if (result != 0 || fileop.fAnyOperationsAborted)
        return 0;

    return RemoveDirectoryA(path) != 0;
}
#endif



bool32 avDirectoryDelete(AvString location, AvDirectoryDeleteOptions options) {
    if(avStringIsEmpty(location)){
        return 0;
    }

    AvString tmp = {0};
    avStringClone(&tmp, location);
    bool32 ret = FALSE;
#ifdef _WIN32

    if (options & AV_DIRECTORY_DELETE_RECURSIVE) {
        ret = avDirectoryDeleteRecursiveWin32(tmp.chrs);
        avStringFree(&tmp);
        return ret;
    }
    ret = RemoveDirectoryA(tmp.chrs) != 0;
    avStringFree(&tmp);
    return ret;

#else
    if (options & AV_DIRECTORY_DELETE_RECURSIVE) {
        ret = avDirectoryDeleteRecursivePosix(location);
        avStringFree(&tmp);
        return ret;
    }

    ret = rmdir(location) == 0;
    avStringFree(&tmp);
    return ret;
#endif
}