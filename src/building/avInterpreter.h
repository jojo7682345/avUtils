#ifndef __AV_INTERPRETER__
#define __AV_INTERPRETER__
#include <AvUtils/avDefinitions.h>
C_SYMBOLS_START
#include <AvUtils/avTypes.h>
#include <AvUtils/dataStructures/avDynamicArray.h>

typedef struct AvCompileData_T {
    AV_DS(AvDynamicArray, Token) tokens;
}AvCompileData_T;

C_SYMBOLS_END
#endif//__AV_INTERPRETER__