#ifndef __AV_DYNAMIC_ARRAY__
#define __AV_DYNAMIC_ARRAY__
#include "../avDefinitions.h"
C_SYMBOLS_START


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

/// <summary>
/// reserves additional space. Trim the array first to make sure the reserved data is contiquous for better performance.
/// </summary>
/// <param name="count"></param>
/// <param name="dynamicArray"></param>
void avDynamicArrayReserve(uint32 count, AvDynamicArray dynamicArray);

uint32 avDynamicArrayWriteRange(void* data, uint32 count, uint64 offset, uint64 stride, uint32 startIndex, AvDynamicArray dynamicArray);
uint32 avDynamicArrayReadRange(void* data, uint32 count, uint64 offset, uint64 stride, uint32 startIndex, AvDynamicArray dynamicArray);

void avDynamicArraySetDeallocateElementCallback(AvDeallocateElementCallback callback, AvDynamicArray dynamicArray);

void avDynamicArraySetGrowSize(uint32 size, AvDynamicArray dynamicArray);
uint32 avDynamicArrayGetGrowSize(AvDynamicArray dynamicArray);

uint32 avDynamicArrayGetSize(AvDynamicArray dynamicArray);
uint32 avDynamicArrayGetCapacity(AvDynamicArray dynamicArray);
uint64 avDynamicArrayGetDataSize(AvDynamicArray dynamicArray);

void avDynamicArrayTrim(AvDynamicArray dynamicArray);
void avDynamicArrayMakeContiguous(AvDynamicArray dynamicArray);

/// <summary>
/// Appends the source array to the destination array. Transfers ownership of all the data within the source array.
/// The source array is practically destroyed after calling this function.
/// Arrays must have the same datasize
/// </summary>
void avDynamicArrayAppend(AvDynamicArray dst, AvDynamicArray* src);


void avDynamicArrayClone(AvDynamicArray src, AvDynamicArray* dynamicArray);



#endif//__AV_DYNAMIC_ARRAY__

#ifdef AV_DYNAMIC_ARRAY_EXPOSE_MEMORY_LAYOUT
#ifndef __AV_DYNAMIC_ARRAY_MEMORY_LAYOUT__
#define __AV_DYNAMIC_ARRAY_MEMORY_LAYOUT__

uint32 avDynamicArrayGetPageCount(AvDynamicArray dynamicArray);
uint32 avDynamicArrayGetPageSize(uint32 pageNum, AvDynamicArray dynamicArray);
uint32 avDynamicArrayGetPageCapacity(uint32 pageNum, AvDynamicArray dynamicArray);
void* avDynamicArrayGetPageDataPtr(uint32 pageNum, AvDynamicArray dynamicArray);
uint64 avDynamicArrayGetPageDataSize(uint32 pageNum, AvDynamicArray dynamicArray);

#define AV_DYNAMIC_ARRAY_INVALID_PAGE ((uint32)-1)
uint32 avDynamicArrayGetIndexPage(uint32* index, AvDynamicArray dynamicArray);
void* avDynamicArrayGetPtr(uint32 index, AvDynamicArray dynamicArray);

#endif//__AV_DYNAMIC_ARRAY_MEMORY_LAYOUT__

C_SYMBOLS_END
#endif