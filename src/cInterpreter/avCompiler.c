#include <AvUtils/avBuilder.h>
#include <AvUtils/avFileSystem.h>
#include <AvUtils/avString.h>
#include <AvUtils/avDataStructures.h>
#include <AvUtils/avLogging.h>
#include <AvUtils/avMemory.h>
#include <AvUtils/string/avChar.h>
#include <stdio.h>

#include "avTokenizer.h"
#include "avInterpreter.h"
#include "avSyntaxTree.h"

AvCompileData createCompileData() {
    return avCallocate(1, sizeof(AvCompileData_T), "allocating compiledata");
}

void destroyCompileData(AvCompileData data) {
    avDynamicArrayDestroy(data->tokens);
    avFree(data);
}

AvParseResult avCompileBuildFile(AvString fileName) {
    AvCompileData data = createCompileData();

    AvParseResult result = AV_PARSE_RESULT_SUCCESS;
    AvFile file = AV_EMPTY;

    avFileHandleCreate(fileName, &file);
    if (avFileOpen(file, (AvFileOpenOptions) { .binary = true, .update = false, .openMode = AV_FILE_OPEN_READ }) == false) {
        result = AV_FILE_NOT_FOUND;
        goto fileReadExit;
    }
    AvStringMemory memory = AV_EMPTY;
    avStringMemoryAllocate(avFileGetSize(file), &memory);
    avFileRead(memory.data, memory.capacity, file);
    avFileClose(file);

    //AvString unprocessedFileData = AV_EMPTY;
    
    AvString fileData = AV_EMPTY;
    
    avStringFromMemory(&fileData, AV_STRING_WHOLE_MEMORY, &memory);

    if (avTokenize(fileData, data) != AV_PARSE_RESULT_SUCCESS) {
        result = AV_PARSE_RESULT_ERROR;
        goto parseExit;
    }

    if (avBuildSyntaxTree(data)){

    }

parseExit:
    avStringFree(&fileData);
fileReadExit:
    avFileHandleDestroy(file);
    destroyCompileData(data);
    return result;
}