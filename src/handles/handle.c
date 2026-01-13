#include "handle.h"

#if UINTPTR_MAX < UINT64_MAX
#error "This allocator requires a 64-bit address space"
#endif

static void* osReserve(uint64 size);
static void* osCommit(void* addr, uint64 size);
static void  osDecommit(void* addr, uint64 size);
static void  osRelease(void* addr, uint64 size);
static uint32 osPageSize();
static uint32 osReserveSize();

static uint32 pageSize;
static uint32 reserveSize;
static uint32 handlePagesPerOsPage;


#ifdef _WIN32

#include <windows.h>

static void* osReserve(uint64 size){
    return VirtualAlloc(NULL, size, MEM_RESERVE, PAGE_NOACCESS);
}
static void* osCommit(void* addr, uint64 size){
    return VirtualAlloc(addr, size, MEM_COMMIT, PAGE_READWRITE);
}
static void  osDecommit(void* addr, uint64 size){
    VirtualFree(addr, size, MEM_DECOMMIT);
}
static void  osRelease(void* addr, uint64 size){
    (void)size;
    VirtualFree(addr, 0, MEM_RELEASE);
}
static uint32 osPageSize(){
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    return (uint32)si.dwPageSize;
}
static uint32 osReserveSize(){
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    return (uint32)si.dwAllocationGranularity;
}

#else

#include <sys/mman.h>
#include <unistd.h>

static void* osReserve(uint64 size){
    void* p = mmap(NULL, size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    return (p == MAP_FAILED) ? NULL : p;
}
static void* osCommit(void* addr, uint64 size){
    if (mprotect(addr, size, PROT_READ | PROT_WRITE) != 0){
        return NULL;
    }
    return addr;
}
static void  osDecommit(void* addr, uint64 size){
    madvise(addr, size, MADV_DONTNEED);
    mprotect(addr, size, PROT_NONE);
}
static void  osRelease(void* addr, uint64 size){
    munmap(addr, size);
}
static uint32 osPageSize(){
    
    return (uint32)sysconf(_SC_PAGESIZE);;
}
static uint32 osReserveSize(){
    return osPageSize();
}

#endif

struct AvHandle_t{
    uint32 pageIndex : 18;
    uint32 id : 7;
    uint32 readOnly : 1;
    uint32 copyOnWrite : 1;
    uint32 attached : 1;
    uint32 version : 4;
};
struct AvHandleFlags_t {
    uint32 slotVersion : 4;
    uint32 slotType : 6;
    uint32 refCount : 8;
};

struct AvHandleData_t{
    struct AvHandleFlags_t flags;
    uint32 size;
    void* data;
    void (*destructor)(avHandle self, void* data, uint32 size); // included data and size for fast access
    avHandle first; // last child is stored in the last field of the first child
    uint16 type;
#ifdef NDEBUG
    uint16 padding;
#else
    const char* debugLocation;
#endif
};

struct AvAttachedHandleData_t{ // fits in 64 bytes (48)
    struct AvHandleData_t base;
    avHandle parent; //
    avHandle next; // first sibling
    avHandle last; // last sibling
#ifdef NDEBUG
    uint64 padding[2];
#else
    uint64 padding[1];
#endif
};
struct AvOrphanHandleData_t{ // fits in 32 bytes (28)
    struct AvHandleData_t base;
#ifndef NDEBUG
    uint64 padding[3];
#endif
};

#define KiB (1024ULL)
#define MiB (1024ULL * KiB)
#define GiB (1024ULL * MiB)
#define TiB (1024ULL * GiB)

#define HANDLE_BASE_SIZE (2ULL * GiB) // maximum number of handles in size 18-bit page index * 7 bit handle index * 64 bytes in size
#define DATA_BASE_SIZE (8ULL * TiB) 
static void* handleBase;
static void* dataBase;

static inline void* align_up(void* x, uint64 align) {
    return (void*)(((uint64)x + align - 1) & ~(align - 1));
}

enum AvHandlePageType{
    AV_HANDLE_PAGE_TYPE_NORMAL,
    AV_HANDLE_PAGE_TYPE_ORHPAN,
};

typedef struct AvHandlePage{
    uint64 bitMaskL;
    uint64 bitMaskH;
    uint32 freeCount;
    enum AvHandlePageType type;
    struct AvHandlePage* nextAnyFree; // NULL indicates all allocated pages are full, and a new page needs to be allocated
    uint64 padding[4];
    union{
        struct AvAttachedHandleData_t attached[(4096-sizeof(struct AvAttachedHandleData_t))/sizeof(struct AvAttachedHandleData_t)];
        struct AvOrphanHandleData_t orphan[(4096-sizeof(struct AvAttachedHandleData_t))/sizeof(struct AvOrphanHandleData_t)];
    };
} AvHandlePage;

__attribute__((constructor))
__attribute__((used))
static void avUtilsInit(void){
    pageSize = osPageSize();
    reserveSize = osReserveSize();

    handlePagesPerOsPage = sizeof(AvHandlePage)/pageSize;

    handleBase = align_up(osReserve(HANDLE_BASE_SIZE), pageSize);
    dataBase = align_up(osReserve(DATA_BASE_SIZE), pageSize);
}

__attribute__((destructor))
__attribute__((used))
static void avUtilsDeinit(void){
    osRelease(handleBase, HANDLE_BASE_SIZE);
    osRelease(dataBase, DATA_BASE_SIZE);
}

AvHandlePage* pageThatHasAnyFree = NULL;


static AvHandlePage* allocateNewHandlePage(){
    const uint32 numPages = handlePagesPerOsPage;
    AvHandlePage newPages[numPages];
    return newPages;

}

avHandle avAllocate_(uint32 size, AV_DEBUG_LOCATION_PARAMS, avHandle parent){
    if(!pageThatHasAnyFree){
        pageThatHasAnyFree = allocateNewHandlePage();        
    }


}

void avFree(avHandle handle){

}

avHandle avClone_(avHandle handle, AV_DEBUG_LOCATION_PARAMS, avHandle parent){

}

bool8 avHandleGetPtr(avHandle handle, void** ptr, uint32* size){

}