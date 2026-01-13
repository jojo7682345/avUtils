#include <AvUtils/memory/avDynamicAllocator.h>
#include <AvUtils/avMemory.h>
#include <AvUtils/avLogging.h>
#define AV_DYNAMIC_ARRAY_EXPOSE_MEMORY_LAYOUT
#include <AvUtils/dataStructures/avDynamicArray.h>
#include <AvUtils/avMath.h>

#define AV_DYNAMIC_ALLOCATOR_ALIGNMENT 8
#define AV_DYNAMIC_ALLOCATOR_INCREMENT_SIZE (1<<10)

static void allocatePage(uint64 size, AvDynamicAllocator* allocator){
    struct AvDynamicAllocatorPage* page = (struct AvDynamicAllocatorPage*)avCallocate(1, sizeof(struct AvDynamicAllocatorPage) + size, "");
    page->start = (byte*)page + sizeof(struct AvDynamicAllocatorPage);
    page->current = page->start;
    page->remainingSize = size;
    page->previous = allocator->current;
    allocator->current = page;

}

static uint64 alignUp(uint64 size) {
    return (size + (AV_DYNAMIC_ALLOCATOR_ALIGNMENT - 1))
           & ~(AV_DYNAMIC_ALLOCATOR_ALIGNMENT - 1);
}

void avDynamicAllocatorCreate(uint64 initialPageSize, AvDynamicAllocator* allocator) {
    
    allocator->current = NULL;
    allocator->totalAllocatedSize = 0;
    if (initialPageSize) {
        allocatePage(AV_MAX(nextPow2(alignUp(initialPageSize)), AV_DYNAMIC_ALLOCATOR_INCREMENT_SIZE), allocator);
    }
}

static void* returnAllocatedSpace(uint64 size, struct AvDynamicAllocatorPage* page, AvDynamicAllocator* allocator){
    void* ptr = page->current;
    page->current = (byte*)page->current + size;
    page->remainingSize -= size;
    allocator->totalAllocatedSize += size;  
    return ptr;
}

void* avDynamicAllocatorAllocate(uint64 size, AvDynamicAllocator* allocator) {
    avAssert(size != 0, "cannot allocate of size 0");

    size = alignUp(size);

    if(!allocator->current || size > allocator->current->remainingSize){
        if(size > AV_DYNAMIC_ALLOCATOR_INCREMENT_SIZE){
            allocatePage(nextPow2(size), allocator);
        }else{
            allocatePage(AV_DYNAMIC_ALLOCATOR_INCREMENT_SIZE, allocator);
        }
    }
    return returnAllocatedSpace(size, allocator->current, allocator);
}

uint64 avDynamicAllocatorGetAllocatedSize(AvDynamicAllocator* allocator){
    return allocator->totalAllocatedSize;
}

void avDynamicAllocatorReset(AvDynamicAllocator* allocator) {
    
    // free everything but the largest page

    struct AvDynamicAllocatorPage* page = allocator->current;
    if(!page){
        allocator->totalAllocatedSize = 0;
        return;
    }

    struct AvDynamicAllocatorPage* largestPage = page;
    uint64 largestPageSize = largestPage->current - largestPage->start + largestPage->remainingSize;

    while(page){
        struct AvDynamicAllocatorPage* next = page->previous;
        uint64 pageSize = page->remainingSize + (uint64)((byte*)page->current - (byte*)page->start);

        if(pageSize > largestPageSize){
            avFree(largestPage);
            largestPage = page;
            largestPageSize = pageSize;
        }else{
            avFree(page);
        }
        page = next;
    }
    largestPage->current = largestPage->start;
    largestPage->remainingSize = largestPageSize;
    largestPage->previous = NULL;
    
    allocator->totalAllocatedSize = 0;
    allocator->current = largestPage;
}

void avDynamicAllocatorDestroy(AvDynamicAllocator* allocator) {
    struct AvDynamicAllocatorPage* page = allocator->current;

    while(page){
        struct AvDynamicAllocatorPage* next = page->previous;
        avFree(page);
        page = next;
    }

    allocator->current = NULL;
    allocator->totalAllocatedSize = 0;
}

void avDynamicAllocatorReadAll(void* data, AvDynamicAllocator allocator){
    avAssert(data!=NULL, "there must be a destination");

    if(!data || !allocator.current){
        return;
    }

    uint32 pageCount = 0;
    struct AvDynamicAllocatorPage* page = allocator.current;
    while(page){
        pageCount++;
        page = page->previous;
    }

    struct AvDynamicAllocatorPage* pages[pageCount];
    page = allocator.current;
    for(uint64 i = 0; i < pageCount; i++){
        pages[i] = page;
        page = page->previous;
    }

    byte* dst = (byte*) data;
    for(uint64 i = (uint64)pageCount; i > 0; i--){
        struct AvDynamicAllocatorPage* p = pages[i];
        uint64 usedSize = (uint64)((byte*)p->current - (byte*)(p->start));
        if(usedSize > 0){
            avMemcpy(dst, p->start, usedSize);
            dst += usedSize;
        }
    }
}
