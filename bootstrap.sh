#ifdef __BOOTSTRAP_CIRCUIT__ 
#!/bin/sh
echo compiling bootstrap mechanism
mkdir bootstrap
cd bootstrap
self = $(basename "$0")
gcc - o tmp - x c $self -Wl,-rpath,.
echo compiled bootstrap mechanism
echo bootstrapping
run . / tmp $@
ret = $ ?
if[$ret == 0]; then
echo boostrapping successfull
else
echo bootstrapping failed
fi
rm tmp
cd ..
rmdir bootstrap
exit $ret
#endif

#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include "include/AvUtils/avTypes.h"

#define CC "gcc"
#define CFLAGS "-std=c11 -Wpedantic -ggdb -fPIC"
#define INCLUDES "include"

typedef struct File {
    const char* output;
    const char* source;
} File;

#define SOURCE_FILE(dir, file) (File){ file ".o", dir "/" file ".c" }

int compileFile(File file, unsigned int index, unsigned int max) {
    char buffer[2048] = { 0 };
    sprintf_s(buffer, 2048, "%s %s -c -o %s ../%s -I../include", CC, CFLAGS, file.output, file.source);
    int result = system(buffer);
    printf("[%u/%u] compiling %s\n", index, max, file.source);
    return result;
}

int linkFile(const char* output, unsigned int fileCount, const File files[], unsigned int index, unsigned int max){
    char buffer[2048] = { 0 };
    char objects[1024] = { 0 };
    
    int offset = 0;
    for(uint i = 0; i < fileCount; i++){
        offset += sprintf_s(objects + offset, 1024 - offset, "%s ", files[i].output);
    }

    sprintf_s(buffer, 2048, "%s %s -o ../%s %s ", CC, CFLAGS, output, objects);
    int result = system(buffer);
    printf("[%u/%u] linking %s\n", index, max, output);
    return result;
}


int main(int argC, const char* argV[]) {
    bool32 failed = false;
    const File bootstrapFiles[] = {
        SOURCE_FILE("src/threading",        "avThread"),
        SOURCE_FILE("src/threading",        "avMutex"),
        SOURCE_FILE("src/memory",           "avMemory"),
        SOURCE_FILE("src/processes",        "avProcess"),
        SOURCE_FILE("src/logging",          "avAssert"),
        SOURCE_FILE("src/dataStructures",   "avDynamicArray"),
        SOURCE_FILE("src/dataStructures",   "avArray"),
        SOURCE_FILE("src/string",           "avChar"),
        SOURCE_FILE("src/string",           "avString"),
        SOURCE_FILE("src/filesystem",       "avFile"),
        SOURCE_FILE("src/building",         "avBuilder"),
    };
    const uint bootstrapFilesCount = sizeof(bootstrapFiles)/sizeof(File);
    for(uint i = 0; i < bootstrapFilesCount; i++){
        if(compileFile(bootstrapFiles[i],i+1,bootstrapFilesCount+1)!=0){
            failed = true;
        }
    }
    if(linkFile("avBuild", bootstrapFilesCount, bootstrapFiles, bootstrapFilesCount+1, bootstrapFilesCount + 1)!=0){
        failed = true;
    }
    return failed;
}
