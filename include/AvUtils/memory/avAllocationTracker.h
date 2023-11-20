#ifndef __AV_MEMORY_MANAGEMENT__
#define __AV_MEMORY_MANAGEMENT__

#include "../avTypes.h"
#include "../dataStructures/avDynamicArray.h"

typedef struct AvLocationData {
	void* allocation;
	uint64 line;
	const char* function;
	const char* file;
}AvLocationData;

extern struct AvAllocationTracker {
	AvDynamicArray trackedAllocations;
} avAllocationTracker;

void initAllocationTracker(struct AvAllocationTracker* tracker);

void avAllocationTrackerPrintAll();

void deinitAllocationTracker();

#endif//__AV_MEMORY_MANAGEMENT__