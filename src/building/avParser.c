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


enum { STATEMENT_TYPE_COUNTER_BASE = __COUNTER__ };
#define STATEMENT_TYPE_LOCAL_COUNTER (__COUNTER__ - STATEMENT_TYPE_COUNTER_BASE - 1)
#define COUNTER STATEMENT_TYPE_LOCAL_COUNTER
typedef enum StatementType {
    STATEMENT_TYPE_NONE = 0,
    STATEMENT_TYPE_FUNCTION = 1 << COUNTER,
    STATEMENT_TYPE_RETURN = 1 << COUNTER,
    STATEMENT_TYPE_IF = 1 << COUNTER,
    STATEMENT_TYPE_VALUE = 1 << COUNTER,
    STATEMENT_TYPE_CALL = (1 << COUNTER) | STATEMENT_TYPE_VALUE,
    STATEMENT_TYPE_EXPRESSION = (1 << COUNTER) | STATEMENT_TYPE_VALUE,
    STATEMENT_TYPE_FILE = (1 << COUNTER),
    STATEMENT_TYPE_STRING = (1 << COUNTER) | STATEMENT_TYPE_VALUE,

    STATEMENT_TYPE_All = (-1)
} StatementType;
#undef COUNTER

static void checkSyntax(int a, ...){
    
}

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

    AvString unprocessedFileData = AV_EMPTY;
    AvString fileData = AV_EMPTY;
    
    avStringFromMemory(&unprocessedFileData, AV_STRING_WHOLE_MEMORY, &memory);
    avStringReplace(&fileData, unprocessedFileData, AV_CSTR("\r\n"), AV_CSTR("\n"));
    avStringFree(&unprocessedFileData);

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