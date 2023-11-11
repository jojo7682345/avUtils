#ifndef __AV_DYNAMIC_ARRAY__
#define __AV_DYNAMIC_ARRAY__

#include "../avTypes.h"

typedef struct AvDynamicArray_T* AvDynamicArray;

#define AV_DYNAMIC_ARRAY_DEFAULT_GROW_SIZE 8

bool32 avDynamicArrayCreate(uint32 initialSize, uint64 dataSize, AvDynamicArray* dynamicArray);

void avDynamicArrayDestroy(AvDynamicArray dynamicArray);

uint32 avDynamicArrayAdd(void* data, AvDynamicArray dynamicArray);
bool32 avDynamicArrayWrite(void* data, uint32 index, AvDynamicArray dynamicArray);
bool32 avDynamicArrayRead(void* data, uint32 index, AvDynamicArray dynamicArray);

bool32 avDynamicArrayRemove(uint32 index, AvDynamicArray dynamicArray);

void avDynamicArrayClear(void* data, AvDynamicArray dynamicArray);

void avDynamicArrayReserve(uint32 count, AvDynamicArray dynamicArray);

uint32 avDynamicArrayWriteRange(void* data, uint32 count, uint64 offset, uint64 stride, uint32 index, AvDynamicArray dynamicArray);
uint32 avDynamicArrayReadRange(void* data, uint32 count, uint64 offset, uint64 stride, uint32 index, AvDynamicArray dynamicArray);

void avDynamicArraySetGrowSize(uint32 size, AvDynamicArray dynamicArray);
uint32 avDynamicArrayGetGrowSize(AvDynamicArray dynamicArray);

uint32 avDynamicArrayGetSize(AvDynamicArray dynamicArray);
uint32 avDynamicArrayGetCapacity(AvDynamicArray dynamicArray);
uint64 avDynamicArrayGetDataSize(AvDynamicArray dynamicArray);

void avDynamicArrayTrim(AvDynamicArray dynamicArray);

#ifdef AV_DYNAMIC_ARRAY_EXPOSE_MEMORY_LAYOUT

uint32 avDynamicArrayGetPageCount(AvDynamicArray dynamicArray);
uint32 avDynamicArrayGetPageSize(uint32 pageNum, AvDynamicArray dynamicArray);
uint32 avDynamicArrayGetPageCapacity(uint32 pageNum, AvDynamicArray dynamicArray);
void* avDynamicArrayGetPageDataPtr(uint32 pageNum, AvDynamicArray dynamicArray);
uint64 avDynamicArrayGetPageDataSize(uint32 pageNum, AvDynamicArray dynamicArray);

#endif

#endif//__DYNAMIC_ARRAY__