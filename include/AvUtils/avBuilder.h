#ifndef __AV_BUILDER__
#define __AV_BUILDER__
#include "avDefinitions.h"
C_SYMBOLS_START
#include "avTypes.h"
#include "avString.h"

typedef bool32 (*avConfigCallback)(void);

void avBuildSetup();
void avBuildRegisterConfiguration(AvString str, avConfigCallback configCallback);

C_SYMBOLS_END
#endif//__AV_BUILDER__