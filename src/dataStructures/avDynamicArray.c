#define AV_DYNAMIC_ARRAY_EXPOSE_MEMORY_LAYOUT
#include <AvUtils/dataStructures/avDynamicArray.h>

#include <AvUtils/avMemory.h>
#include <AvUtils/avLogging.h>

#include <stdio.h>
#include <string.h>

typedef struct Page {
	void* data;
	uint32 count;
	uint32 capacity;
	struct Page* next;
	struct Page* prev;
} Page;

typedef struct AvDynamicArray_T {
	Page* data;
	Page* lastPage;
	uint32 pageCount;

	uint64 dataSize;
	uint32 growSize;

	uint32 count;
	uint32 capacity;

	AvDeallocateElementCallback deallocElement; 
} AvDynamicArray_T;

static Page* createPage(uint32 capacity, uint64 dataSize) {
	Page* page = avCallocate(1, sizeof(Page), "allocating page");
	page->data = avCallocate(capacity, dataSize, "allocating page data");
	page->count = 0;
	page->capacity = capacity;
	page->next = nullptr;
	page->prev = nullptr;
	return page;
}

static void destroyPage(Page* page) {
	avFree(page->data);
	avFree(page);
}

static Page* addPage(uint32 count, AvDynamicArray dynamicArray) {
	if (count == 0) {
		return nullptr;
	}
	Page* page;
	page = createPage(count, dynamicArray->dataSize);
	dynamicArray->capacity += count;
	page->prev = dynamicArray->lastPage;
	if (dynamicArray->lastPage) {
		dynamicArray->lastPage->next = page;
	}
	if(dynamicArray->data == nullptr){
		dynamicArray->data = page;
	}
	dynamicArray->lastPage = page;
	dynamicArray->pageCount++;
	return page;
}

static void removePage(Page* page, AvDynamicArray dynamicArray) {
	if (!page) {
		return;
	}
	if (page == dynamicArray->data) {
		dynamicArray->data = page->next;
	}

	if (page == dynamicArray->lastPage) {
		dynamicArray->lastPage = page->prev;
	}

	if (page->next) {
		page->next->prev = page->prev;
	}
	if (page->prev) {
		page->prev->next = page->next;
	}

	dynamicArray->count -= page->count;
	dynamicArray->capacity -= page->capacity;
	destroyPage(page);
	dynamicArray->pageCount--;
}

static void resizePage(Page* page, uint32 capacity, AvDynamicArray dynamicArray) {

	if (capacity == 0) {
		removePage(page, dynamicArray);
	}

	int64 capacityDiff = (int64)page->capacity - (int64)capacity;
	dynamicArray->capacity -= capacityDiff;
	uint32 count = page->count < capacity ? page->count : capacity;

	int64 countDiff = (uint64)page->count - (int64)count;
	dynamicArray->count -= countDiff;

	page->count = count;
	page->capacity = capacity;

	page->data = avReallocate(page->data, (uint64)page->capacity * dynamicArray->dataSize, "reallocating page data");
}

bool32 avDynamicArrayCreate(uint32 initialSize, uint64 dataSize, AvDynamicArray* dynamicArray) {
	if (dataSize == 0) {
		avAssert(dataSize == 0, "invalid parameters");
		return 0;
	}

	(*dynamicArray) = avCallocate(1, sizeof(AvDynamicArray_T), "allocating dynamic array handle");
	(*dynamicArray)->dataSize = dataSize;
	(*dynamicArray)->growSize = AV_DYNAMIC_ARRAY_DEFAULT_GROW_SIZE;
	(*dynamicArray)->count = 0;
	(*dynamicArray)->capacity = 0;
	(*dynamicArray)->data = addPage(initialSize, *dynamicArray);
	(*dynamicArray)->deallocElement = nullptr;
	return 1;
}

static void* getPtr(Page* page, uint32 index, AvDynamicArray dynamicArray) {
	return (byte*)page->data + (uint64)index * dynamicArray->dataSize;
}

void avDynamicArrayDestroy(AvDynamicArray dynamicArray) {
	if (dynamicArray == nullptr) {
		return;
	}
	Page* page = dynamicArray->data;
	while (page) {

		if (dynamicArray->deallocElement) {
			for (uint i = 0; i < page->count; i++) {
				dynamicArray->deallocElement(getPtr(page, i, dynamicArray), dynamicArray->dataSize);
			}
		}

		Page* nextPage = page->next;
		destroyPage(page);
		page = nextPage;
	}
	avFree(dynamicArray);
}



static Page* getPage(uint32* index, AvDynamicArray dynamicArray) {
	Page* page = dynamicArray->data;

	while (page) {
		if (*index >= page->capacity) {
			(*index) -= page->capacity;
			page = page->next;
		} else {
			return page;
		}
		if (page == NULL) {
			*index = 0;
			return NULL;
		}
	}

	return page;
}

uint32 avDynamicArrayAdd(void* data, AvDynamicArray dynamicArray) {
	avAssert(data != NULL, "data cannot be a null pointer");

	uint32 index = dynamicArray->count;
	Page* page = getPage(&index, dynamicArray);
	if (page == NULL) {
		page = addPage(dynamicArray->growSize, dynamicArray);
	}

	memcpy(getPtr(page, index, dynamicArray), data, dynamicArray->dataSize);
	page->count++;
	return dynamicArray->count++;

}

bool32 avDynamicArrayWrite(void* data, uint32 index, AvDynamicArray dynamicArray) {

	Page* page = getPage(&index, dynamicArray);

	if (page == NULL) {
		return false;
	}

	if (index >= page->count) {
		return false;
	}

	memcpy(getPtr(page, index, dynamicArray), data, dynamicArray->dataSize);

	return true;
}

bool32 avDynamicArrayRead(void* data, uint32 index, AvDynamicArray dynamicArray) {

	Page* page = getPage(&index, dynamicArray);

	if (page == NULL) {
		return false;
	}

	if (index >= page->count) {
		return false;
	}

	memcpy(data, getPtr(page, index, dynamicArray), dynamicArray->dataSize);

	return true;
}

bool32 avDynamicArrayRemove(uint32 index, AvDynamicArray dynamicArray) {
	if (index >= dynamicArray->count) {
		return false;
	}

	Page* page = getPage(&index, dynamicArray);
	if (page == NULL) {
		return false;
	}
	if (index >= page->count) {
		return false;
	}

	if (dynamicArray->deallocElement) {
		dynamicArray->deallocElement(getPtr(page, index, dynamicArray), dynamicArray->dataSize);
	}

	uint32 followingCount = page->count - index - 1;

	memmove(getPtr(page, index, dynamicArray), getPtr(page, index + 1, dynamicArray), (uint64)followingCount * dynamicArray->dataSize);

	if (page->next && page->next->count != 0) {
		resizePage(page, page->count-1, dynamicArray);
	}else{
		page->count--;
		dynamicArray->count--;
	}
}

static void clearPage(Page* page, void* data, AvDynamicArray dynamicArray) {

	if (data) {
		for (uint32 i = 0; i < page->capacity; i++) {
			memcpy(getPtr(page, i, dynamicArray), data, dynamicArray->dataSize);
		}
	} else {
		memset(page->data, 0, dynamicArray->dataSize * (uint64)page->capacity);
	}

	page->count = 0;
}

void avDynamicArrayClear(void* data, AvDynamicArray dynamicArray) {

	Page* page = dynamicArray->data;
	while (page) {
		clearPage(page, data, dynamicArray);
		page = page->next;
	}

}

void avDynamicArrayReserve(uint32 count, AvDynamicArray dynamicArray) {

	uint32 index = dynamicArray->count;
	Page* page = getPage(&index, dynamicArray);
	if (page) {
		uint32 remainingSpace = page->capacity - page->count;
		avAssert(remainingSpace != 0, "this should only occur when I programmed it wrong");
		if (count <= remainingSpace) {
			return;
		}
		count -= remainingSpace;
	}
	addPage(count, dynamicArray);
}

uint32 avDynamicArrayWriteRange(void* data, uint32 count, uint64 offset, uint64 stride, uint32 startIndex, AvDynamicArray dynamicArray) {
	if (count == 0) {
		return 0;
	}
	byte* dataPtr = (byte*)data + offset;
	for (uint i = 0; i < count; i++) {
		if (!avDynamicArrayWrite(dataPtr, startIndex + i, dynamicArray)) {
			return i + 1;
		}
		dataPtr += stride;
	}
	return count;
}

uint32 avDynamicArrayReadRange(void* data, uint32 count, uint64 offset, uint64 stride, uint32 startIndex, AvDynamicArray dynamicArray) {
	if (count == 0) {
		return 0;
	}

	byte* dataPtr = (byte*)data + offset;
	for (uint i = 0; i < count; i++) {
		if (!avDynamicArrayRead(data, startIndex + i, dynamicArray)) {
			return i + 1;
		}
		dataPtr += stride;
	}
	return count;
}

void avDynamicArraySetDeallocateElementCallback(AvDeallocateElementCallback callback, AvDynamicArray dynamicArray) {
	dynamicArray->deallocElement = callback;
}

void avDynamicArraySetGrowSize(uint32 size, AvDynamicArray dynamicArray) {
	dynamicArray->growSize = size;
}

uint32 avDynamicArrayGetGrowSize(AvDynamicArray dynamicArray) {
	return dynamicArray->growSize;
}

uint32 avDynamicArrayGetSize(AvDynamicArray dynamicArray) {
	return dynamicArray->count;
}

uint32 avDynamicArrayGetCapacity(AvDynamicArray dynamicArray) {
	return dynamicArray->capacity;
}

uint64 avDynamicArrayGetDataSize(AvDynamicArray dynamicArray) {
	return dynamicArray->dataSize;
}

void avDynamicArrayTrim(AvDynamicArray dynamicArray) {

	Page* page = dynamicArray->lastPage;
	while (page) {
		if (page->count == page->capacity) {
			return;
		}
		if (page->count == 0) {
			Page* nextPage = page->prev;
			removePage(page, dynamicArray);
			page = nextPage;
		} else {
			resizePage(page, page->count, dynamicArray);
			return;
		}
	}
}

void avDynamicArrayAppend(AvDynamicArray dst, AvDynamicArray* src) {
	if (dst->dataSize != (*src)->dataSize) {
		return;
	}
	avDynamicArrayTrim(dst);
	dst->lastPage->next = (*src)->data;
	(*src)->data->prev = dst->lastPage;
	dst->count += (*src)->count;
	dst->capacity += (*src)->capacity;
	(*src)->data = nullptr;
	avDynamicArrayDestroy(*src);
	*src = nullptr;
}

uint32 avDynamicArrayGetPageCount(AvDynamicArray dynamicArray) {
	return dynamicArray->pageCount;
}

static Page* findPage(uint32 pageNum, AvDynamicArray dynamicArray) {
	if (pageNum >= dynamicArray->pageCount) {
		return 0;
	}
	Page* page = dynamicArray->data;
	uint32 index = 0;
	while (index < pageNum && page) {
		page = page->next;
		index++;
	}
	return page;
}

uint32 avDynamicArrayGetPageSize(uint32 pageNum, AvDynamicArray dynamicArray) {
	
	Page* page = findPage(pageNum, dynamicArray);
	if (page == NULL) {
		return 0;
	}
	return page->count;
}

uint32 avDynamicArrayGetPageCapacity(uint32 pageNum, AvDynamicArray dynamicArray) {
	Page* page = findPage(pageNum, dynamicArray);
	if (page == NULL) {
		return 0;
	}
	return page->capacity;
}

void* avDynamicArrayGetPageDataPtr(uint32 pageNum, AvDynamicArray dynamicArray) {
	Page* page = findPage(pageNum, dynamicArray);
	if (page == NULL) {
		return 0;
	}
	return page->data;
}

uint64 avDynamicArrayGetPageDataSize(uint32 pageNum, AvDynamicArray dynamicArray) {
	Page* page = findPage(pageNum, dynamicArray);
	if (page == NULL) {
		return 0;
	}
	return (uint64)page->capacity * dynamicArray->dataSize;
}



