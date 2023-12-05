#ifndef __AV_BUILDER__
#define __AV_BUILDER__
#include "avDefinitions.h"
C_SYMBOLS_START
#include "avTypes.h"
#include "avString.h"

typedef bool32 (*avConfigCallback)(void);
typedef struct AvCompileData_T* AvCompileData;

typedef enum AvParseResult {
    AV_PARSE_RESULT_SUCCESS = 0,
    AV_PARSE_RESULT_ERROR,
    AV_PARSE_RESULT_FILE_ERROR,
    AV_PARSE_RESULT_INVALID_TOKEN,
    AV_PARSE_RESULT_SYNTAX_ERROR,
}AvParseResult;

void avBuildSetup();
void avBuildRegisterConfiguration(AvString str, avConfigCallback configCallback);
AvParseResult avCompileBuildFile(AvString fileName);
C_SYMBOLS_END
#endif//__AV_BUILDER__