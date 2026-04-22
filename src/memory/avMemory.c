#undef AV_DEBUG_ALLOC
#include <AvUtils/avMemory.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

typedef struct AvAllocRecord {
    void* ptr;
    uint64 size;
    const char* file;
    const char* func;
    uint line;
    const char* message;
    struct AvAllocRecord* next;
} AvAllocRecord;

static AvAllocRecord* g_allocList = NULL;

static void avTrackAlloc(void* ptr, uint64 size, const char* message,
                        uint line, const char* func, const char* file)
{
    AvAllocRecord* rec = (AvAllocRecord*)malloc(sizeof(AvAllocRecord));
    rec->ptr = ptr;
    rec->size = size;
    rec->file = file;
    rec->func = func;
    rec->line = line;
    rec->message = message;

    rec->next = g_allocList;
    g_allocList = rec;
}

static void avTrackFree(void* ptr)
{
    AvAllocRecord** current = &g_allocList;

    while (*current) {
        if ((*current)->ptr == ptr) {
            AvAllocRecord* toDelete = *current;
            *current = (*current)->next;
            free(toDelete);
            return;
        }
        current = &((*current)->next);
    }

    printf("WARNING: Attempt to free untracked pointer %p\n", ptr);
}

static void avTrackRealloc(void* oldPtr, void* newPtr, uint64 size,
    const char* message, uint line,
    const char* func, const char* file) {
    if (oldPtr) avTrackFree(oldPtr);
    if (newPtr) avTrackAlloc(newPtr, size, message, line, func, file);
}

void avDumpLeaks(void) {
    AvAllocRecord* current = g_allocList;

    if (!current) {
        //printf("No memory leaks detected.\n");
        return;
    }

    printf("Memory leaks detected:\n");

    while (current) {
        printf("Leaked "
#ifdef _WIN32
            "%llu"
#else
            "%lu"
#endif 
            " bytes at %p (%s)\n",
               current->size,
               current->ptr,
               current->message);

        printf("  Location: %s:%u (%s)\n",
               current->file,
               current->line,
               current->func);

        current = current->next;
    }
}

__attribute__((destructor))
__attribute__((used))
static void avDumpLeaksImpl(void){
    avDumpLeaks();
}

void* avAllocate_(uint64 size, const char* message, uint line, const char* func, const char* file) {
	void* data = malloc(size);
	if (!data) {
		printf("malloc returned null: %s\n", message);
		exit(-1);
		return NULL;
	}
	return data;
}

void* avCallocate_(uint64 count, uint64 size, const char* message, uint line, const char* func, const char* file) {
	void* data = calloc(count, size);
	if (!data) {
		printf("calloc returned null: %s\n", message);
		exit(-1);
		return NULL;
	}
	return data;
}

void* avReallocate_(void* data, uint64 size, const char* message, uint line, const char* func, const char* file) {
	if(size==0){
		avFree_(data, line, func, file);
		return NULL;
	}
	void* newPtr = realloc(data, size);
	if (!newPtr) {
		printf("realloc returned null : %s\n", message);
		exit(-1);
		return NULL;
	}
	return newPtr;
}

void avFree_(void* data, uint line, const char* func, const char* file) {
	free(data);
}


void* avAllocateDebug_(uint64 size, const char* message, uint line, const char* func, const char* file){
    void* data = avAllocate_(size, message, line, func, file);
    avTrackAlloc(data, size, message, line, func, file);
    return data;
}

void* avCallocateDebug_(uint64 count, uint64 size, const char* message, uint line, const char* func, const char* file){
    void* data = avCallocate_(count, size, message, line, func, file);
    avTrackAlloc(data, count*size, message, line, func, file);
    return data;
}

void* avReallocateDebug_(void* data, uint64 size, const char* message, uint line, const char* func, const char* file) {
    void* newData = avReallocate_(data, size, message, line, func, file);
    avTrackRealloc(data, newData, size, message, line, func, file);
    return newData;
}

void avFreeDebug_(void* data, uint line, const char* func, const char* file){
    avFree_(data, line, func, file);
    if(data)avTrackFree(data);
}

__attribute__((optimize("O2")))
void avMemcpy(void* restrict dst, const void* restrict src, uint64 size) {
    byte* d = (byte*)dst;
    const byte* s = (const byte*)src;

    while (((uint64)d & 7) && size) {
        *d++ = *s++;
        size--;
    }

    const uint64* s64 = (const uint64*)s;
    uint64* d64 = (uint64*)d;
    while (size >= 8) {
        *d64++ = *s64++;
        size -= 8;
    }

    d = (byte*)d64;
    s = (const byte*)s64;

    while (size--) {
        *d++ = *s++;
    }
}
__attribute__((optimize("O2")))
void avMemset(void* restrict dst, byte value, uint64 size) {
    byte* d = (byte*)dst;

    while (((uint64)d & 7) && size) {
        *d++ = value;
        size--;
    }

    uint64 valueWord = 0x0101010101010101ULL * value;
    uint64* d64 = (uint64*)d;

    while (size >= 8) {
        *d64++ = valueWord;
        size -= 8;
    }

    d = (byte*)d64;
    while (size--) {
        *d++ = value;
    }
}


__attribute__((optimize("O3")))
void avMemswap(void* restrict dst, void* restrict src, uint64 size){
    byte* a = (byte*)dst;
    byte* b = (byte*)src;
    uint64 index = 0;
    while(index < size){
        byte buffer = a[index];
        a[index] = b[index];
        b[index] = buffer;
        index++;
    }
}
