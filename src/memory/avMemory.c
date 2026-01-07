#include <AvUtils/avMemory.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

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

    while (((uintptr_t)d & 7) && size) {
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
