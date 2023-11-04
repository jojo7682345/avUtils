#ifndef __AV_ASSERT__
#define __AV_ASSERT__

#include "../avTypes.h"

#ifndef NDEBUG
#define avAssert(condition, message) (condition ? (void)0 : avAssertFailed(condition, #condition, message, __LINE__,__func__,__FILE__))
#else
#define avAssert(condition, message) ((void)0)
#endif

void avAssertFailed(bool32 condition, const char* expression, const char* message, uint64 line, const char* function, const char* file);

#endif//__AV_ASSERT__