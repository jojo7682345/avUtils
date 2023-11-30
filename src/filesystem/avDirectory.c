#include <AvUtils/filesystem/AvDirectory.h>
#include <AvUtils/avMemory.h>
#include <AvUtils/dataStructures/avDynamicArray.h>
#include <AvUtils/dataStructures/avArray.h>
#include <AvUtils/avLogging.h>

#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <dirent.h>
#endif

typedef struct AvDirectory_T {
    AvPath path;
    AvDirectory parent;
    bool32 explored;
    AV_DS(AvArray, AvDirectoryContent) content;
} AvDirectory_T;

static void listFiles(AvString path, AvDirectory dir);

static void destroyContentElement(AvDirectoryContent* content, uint64 size) {
    switch(content->type){
        case AV_DIRECTORY_CONTENT_TYPE_DIRECTORY:
            avDirectoryClose(&content->directory);
        break;
        case AV_DIRECTORY_CONTENT_TYPE_FILE:
            avFileHandleDestroy(content->file);
        break;
        default:
            printf("invalid type %i\n", content->type);
            break;
    }
}

static void directoryListFiles(AvDirectory dir) {
    avAssert(dir != nullptr, "dir must be a valid reference");
    avAssert(dir->path != nullptr, "dir must have a path");

    AvString directoryPath = AV_EMPTY;
    avPathGetStr(&directoryPath, dir->path);
    avStringAppend(&directoryPath, AV_CSTR("/*"));

    listFiles(directoryPath, dir);
    avStringFree(&directoryPath);
    dir->explored = true;
}

bool32 avDirectoryOpen(AvPath path, AvDirectory* dir) {

    (*dir) = avCallocate(1, sizeof(AvDirectory_T), "allocating directory handle");
    avPathClone(&(*dir)->path, path);
    (*dir)->parent = nullptr;
    (*dir)->explored = false;
    

}

uint32 avDirectoryGetContentCount(AvDirectory dir){
    if(dir->explored == false){
        directoryListFiles(dir);
    }

    return dir->content.count;

}

uint32 avDirectoryRefresh(AvDirectory dir){
    directoryListFiles(dir);
}

AvDirectoryContentType avDirectoryGetContentType(uint32 index, AvDirectory dir){
    if(dir->explored==false){
        directoryListFiles(dir);
    }
    if(index >= dir->content.count){
        return AV_DIRECTORY_CONTENT_TYPE_INVALID;
    }

    AvDirectoryContent content = AV_EMPTY;
    avArrayRead(&content, index, &dir->content);
    return content.type;
}

AvFile avDirectoryOpenFile(uint32 index, AvDirectory dir){
    if (dir->explored == false) {
        directoryListFiles(dir);
    }
    if (index >= dir->content.count) {
        return nullptr;
    }
    AvDirectoryContent content = AV_EMPTY;
    avArrayRead(&content, index, &dir->content);
    if(content.type != AV_DIRECTORY_CONTENT_TYPE_FILE){
        return nullptr;
    }
    return content.file;
}
AvDirectory avDirectoryOpenSubfolder(uint32 index, AvDirectory dir){
    if (dir->explored == false) {
        directoryListFiles(dir);
    }
    if (index >= dir->content.count) {
        return nullptr;
    }
    AvDirectoryContent content = AV_EMPTY;
    avArrayRead(&content, index, &dir->content);
    if (content.type != AV_DIRECTORY_CONTENT_TYPE_DIRECTORY) {
        return nullptr;
    }
    return content.directory;
}

void avDirectoryGetPathStr(AvStringRef str, AvDirectory dir){
    avPathGetStr(str, dir->path);
}

void avDirectoryClose(AvDirectory* dir) {

    avArrayFree(&(*dir)->content);
    avPathDestroy((*dir)->path);

    avFree(*dir);
    *dir = nullptr;
}

static AvDirectoryContent createFile(AvString path, AvString name) {
    AvDirectoryContent content = AV_EMPTY;
    content.type = AV_DIRECTORY_CONTENT_TYPE_FILE;
    AvString str = AV_EMPTY;
    avStringJoin(&str, path, AV_CSTR("/"), name);
    avFileHandleCreate(str, &content.file);
    avStringFree(&str);
    return content;
}

static AvDirectoryContent createDir(AvDirectory parent, AvString name) {
    AvDirectoryContent content = AV_EMPTY;
    content.type = AV_DIRECTORY_CONTENT_TYPE_DIRECTORY;
    AvPath path;
    avPathClone(&path, parent->path);
    avPathChangeDirectory(name, path);
    avDirectoryOpen(path, &content.directory);
    avPathDestroy(path);
    content.directory->parent = parent;
    return content;
}


#ifdef _WIN32

static void listFiles(AvString directoryPath, AvDirectory parent) {

    WIN32_FIND_DATA findFileData = {0};
    HANDLE hFind = FindFirstFile(directoryPath.chrs, &findFileData);
    if (hFind == INVALID_HANDLE_VALUE) {
        return;
    }
    AvDynamicArray list;
    avDynamicArrayCreate(0, sizeof(AvDirectoryContent), &list);
    do {
        if(strcmp(findFileData.cFileName, ".")==0 || strcmp(findFileData.cFileName, "..")==0){
            continue;
        }
        bool32 isDir = findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
        
        AvDirectoryContent content = isDir ? 
            createDir(parent, AV_CSTR(findFileData.cFileName)) :
            createFile(AV_STR(directoryPath.chrs,directoryPath.len-2), AV_CSTR(findFileData.cFileName));

        avDynamicArrayAdd(&content, list);

    } while (FindNextFileA(hFind, &findFileData) != 0);
    FindClose(hFind);
    avArrayAllocateWithFreeCallback(avDynamicArrayGetSize(list), sizeof(AvDirectoryContent), &parent->content, false,(AvDeallocateElementCallback) &destroyContentElement, nullptr);
    for(uint32 i = 0; i < avDynamicArrayGetSize(list); i++){
        AvDirectoryContent content;
        avDynamicArrayRead(&content, i, list);
        avArrayWrite(&content, i, &parent->content);
    }
    avDynamicArrayDestroy(list);
}

#else
static AvArray listFiles(AvPath path);

#endif