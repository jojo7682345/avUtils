#include <AvUtils/filesystem/AvDirectory.h>
#include <AvUtils/avMemory.h>

#define AV_DYNAMIC_ARRAY_EXPOSE_MEMORY_LAYOUT
#include <AvUtils/dataStructures/avDynamicArray.h>

#ifdef _WIN32
#include <windows.h>
#else

#endif

typedef struct AvDirectoryTree_T {
    AvString path;
    AV_DS(AvDynamicArray, AvDirectoryEntry_T) entries;
    AvDirectoryEntry root;
} AvDirectoryTree_T;


void listFiles(AvDirectoryEntry entry);

void deallocateTreeEntry(void* data, uint64 dataSize) {
    AvDirectoryEntry entry = (AvDirectoryEntry)data;
    avStringFree(&entry->path);
    switch(entry->type){
        case AV_DIRECTORY_ENTRY_TYPE_DIRECTORY:
            avDynamicArrayDestroy(entry->directory.contents);
        break;
        case AV_DIRECTORY_ENTRY_TYPE_FILE:
            avFileHandleDestroy(entry->file.fileHandle);
        break;
    }
}

bool32 avDirectoryTreeCreate(AvString rootDir, AvDirectoryTree* tree) {
    (*tree) = avCallocate(1, sizeof(AvDirectoryTree_T), "allocating directory tree handle");
    avDynamicArrayCreate(0, sizeof(AvDirectoryEntry_T), &(*tree)->entries);
    avDynamicArraySetGrowSize(1, (*tree)->entries);
    avDynamicArraySetDeallocateElementCallback(&deallocateTreeEntry, (*tree)->entries);
    avStringCopy(&(*tree)->path, rootDir);
}

static AvDirectoryEntry addEntry(AvDirectoryEntry_T* entry, AvDirectoryTree tree){
    entry->tree = tree;
    uint32 entryIndex = avDynamicArrayAdd(entry, tree->entries);
    return (AvDirectoryEntry) avDynamicArrayGetPtr(entryIndex, tree->entries);
}


static void addDirectoryEntry(struct directory* directory) {
    directory->explored = false;
    avDynamicArrayCreate(0, sizeof(AvDirectoryEntry_T), &directory->contents);
}

static void addFileEntry(AvString path, struct file* file, AvDirectoryEntry root) {
    avFileHandleCreate(path, &file->fileHandle);
}

AvDirectoryEntry avDirectoryTreeGetRootDir(AvDirectoryTree tree) {
    if(tree->root){
        return tree->root;
    }
    AvDirectoryEntry_T dirEnt = {0};
    avStringCopy(&dirEnt.path, tree->path);
    dirEnt.type = AV_DIRECTORY_ENTRY_TYPE_DIRECTORY;
    addDirectoryEntry(&dirEnt.directory);
    dirEnt.directory.explored = false;
    AvDirectoryEntry entry = addEntry(&dirEnt, tree);
    tree->root = entry;
    return entry;
}

static void addContentEntry(AvDirectoryEntryType type, const char* name, AvDirectoryEntry root) {

    struct directory* dir = &root->directory;
    AvString pathSeperator = AV_CSTR("/");
    AvString nameStr=  AV_CSTR(name);

    AvStringHeapMemory memory;
    avStringMemoryHeapAllocate(root->path.len + pathSeperator.len + nameStr.len, &memory);
    avStringMemoryStoreCharArraysVA(memory, root->path, AV_CSTR("/"), AV_CSTR((char*)name));
    
    AvDirectoryEntry_T dirEnt = { 0 };
    dirEnt.type = type;
    avStringFromMemory(&dirEnt.path, 0, AV_STRING_FULL_LENGTH, memory);
    
    switch(type){
        case AV_DIRECTORY_ENTRY_TYPE_DIRECTORY:
            addDirectoryEntry(&dirEnt.directory);
        break;
        case AV_DIRECTORY_ENTRY_TYPE_FILE:
            addFileEntry(dirEnt.path, &dirEnt.file, root);
        break;
    }

    AvDirectoryEntry directoryEntry = addEntry(&dirEnt, root->tree);
    avDynamicArrayAdd(directoryEntry, root->directory.contents);    
}

void avDirectoryExplore(AvDirectoryEntry dir) {
    if (dir->type == AV_DIRECTORY_ENTRY_TYPE_FILE) {
        return;
    }  

    listFiles(dir);

    dir->directory.explored = true;
}

uint32 avDirectoryGetContentCount(AvDirectoryEntry directory){
    if(directory->type!=AV_DIRECTORY_ENTRY_TYPE_DIRECTORY){
        return 0;
    }
    if(!directory->directory.explored){
        return AV_DIRECTORY_UNEXPLORED;
    }
    return avDynamicArrayGetSize(directory->directory.contents);
}

static bool32 isDirectory(AvDirectoryEntry dir){
    return dir->type == AV_DIRECTORY_ENTRY_TYPE_DIRECTORY;
}

void avDirectoryGetContents(AvDirectoryEntry* contents, AvDirectoryEntry dir){
    if(!isDirectory(dir)){
        return;
    }
    if(!dir->directory.explored){
        return;
    }
    if(!contents){
        return;
    }
    avDynamicArrayReadRange(contents, avDynamicArrayGetSize(dir->directory.contents), 0, sizeof(AvDirectoryEntry), 0, dir->directory.contents);
}

void avDirectoryTreeDestroy(AvDirectoryTree tree) {
    avDynamicArrayDestroy(tree->entries);
    avFree(tree);
}

#ifdef _WIN32

void listFiles(AvDirectoryEntry entry) {
    WIN32_FIND_DATA fdFile;
    HANDLE hfind = NULL;

    AvString str;
    avStringJoin(&str, entry->path, AV_CSTR("\\*.*"));
    
    hfind = FindFirstFile(str.chrs, &fdFile);
    if (hfind == INVALID_HANDLE_VALUE) {
        return;
    }

    do {
        if (strcmp(fdFile.cFileName, ".") == 0 || strcmp(fdFile.cFileName, "..")==0) {
            continue;
        }

        AvDirectoryEntryType type =
            ((fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) ?
            AV_DIRECTORY_ENTRY_TYPE_DIRECTORY :
            AV_DIRECTORY_ENTRY_TYPE_FILE;


        addContentEntry(type, fdFile.cFileName, entry);

    } while (FindNextFile(hfind, &fdFile));

    avStringFree(&str);
}

#else

#endif