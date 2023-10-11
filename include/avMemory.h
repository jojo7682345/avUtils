#ifndef __AV_MEMORY__
#define __AV_MEMORY__

#include "avTypes.h"

byte* avAllocate_(size_t size, const char* message, uint line, const char* func, const char* file);
#define avAllocate(size, message) avAllocate_(size, message, __LINE__, __func__, __FILE__)

byte* avCallocate_(size_t count, size_t size, const char* message, uint line, const char* func, const char* file);
#define avCallocate(count, size, message) avCallocate_(count, size, message, __LINE__, __func__, __FILE__)

byte* avReallocate_(byte* data, size_t size, const char* message, uint line, const char* func, const char* file);
#define avReallocate(size, message) avReallocate_(size, message, __LINE__, __func__, __FILE__)

void avFree_(byte* data, uint line, const char* func, const char* file);
#define avFree(data) avFree_(data, __LINE__, __func__, __FILE__)

#endif //__AV_MEMORY__