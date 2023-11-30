#ifndef __AV_PATH__
#define __AV_PATH__
#include "../avDefinitions.h"
C_SYMBOLS_START

#include "../avString.h"



typedef struct AvPath_T* AvPath;

void avPathCreate(AvString pathStr, AvPath* path);
void avPathGetStr(AvStringRef str, AvPath path);
void avPathMakeAbsolute(AvPath path);
void avPathSimplify(AvPath path);

void avPathChangeDirectory(AvString str, AvPath path);
void avPathClone(AvPath* dst, AvPath src);

void avPathDestroy(AvPath path);

C_SYMBOLS_END
#endif//__AV_PATH__