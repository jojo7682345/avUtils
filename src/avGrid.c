#include <avGrid.h>
#include <avMemory.h>
#include <string.h>

typedef struct AvGrid_T {
	byte* data;
	const uint64 elementSize;
	uint32 width;
	uint32 height;
} AvGrid_T;

void avGridCreate(uint64 elementSize, uint32 width, uint32 height, AvGrid* grid) {
	if (elementSize == 0) {
		return;
	}
	if (width == 0 || height == 0) {
		return;
	}

	(*grid) = avAllocate(sizeof(AvGrid_T), "allocating grid handle");

	memcpy((void*)&((*grid)->elementSize), &elementSize, sizeof(elementSize));
	(*grid)->width = width;
	(*grid)->height = height;

	uint64 elementCount = (uint64)width * (uint64)height;

	(*grid)->data = avCallocate(elementCount, elementSize, "allocating grid memory");
}

static bool32 checkBounds(uint32 x, uint32 y, AvGrid grid) {
	return x < grid->width && y < grid->height;
}

static uint64 getIndex(uint32 x, uint32 y, AvGrid grid) {
	return (uint64)y * (uint64)grid->width + (uint64)x;
}

static void* getPtr(uint32 x, uint32 y, AvGrid grid) {
	return grid->data + grid->elementSize * getIndex(x,y,grid);
}

static uint64 getElementCount(AvGrid grid) {
	return (uint64)grid->width * (uint64)grid->height;
}

void avGridWrite(void* data, uint32 x, uint32 y, AvGrid grid) {
	if (!checkBounds(x, y, grid)) {
		return;
	}
	if (data == NULL) {
		return;
	}

	memcpy(getPtr(x, y, grid), data, grid->elementSize);
}

void avGridRead(void* data, uint32 x, uint32 y, AvGrid grid) {
	if (!checkBounds(x, y, grid)) {
		return;
	}
	if (data == NULL) {
		return;
	}
	memcpy(data, getPtr(x, y, grid), grid->elementSize);
}

void avGridClear(void* data, AvGrid grid) {
	if (data == NULL) {
		memset(grid->data, 0, getElementCount(grid) * grid->elementSize);
		return;
	}
	
	for(uint32 y = 0; y < grid->height; y++){
		for (uint32 x = 0; x < grid->width; x++) {
			avGridWrite(data, x, y, grid);
		}
	}
}

void aGridDestroy(AvGrid grid) {
	avFree(grid->data);
	avFree(grid);
}
