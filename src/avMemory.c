#include <avMemory.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

void* avAllocate_(size_t size, const char* message, uint line, const char* func, const char* file) {
	void* data = malloc(size);

	if (!data) {
		printf("malloc returned null: %s\n", message);
		exit(-1);
		return NULL;
	}

	return data;
}

void* avCallocate_(size_t count, size_t size, const char* message, uint line, const char* func, const char* file) {
	void* data = calloc(count, size);

	if (!data) {
		printf("calloc returned null: %s\n", message);
		exit(-1);
		return NULL;
	}

	return data;
}

void* avReallocate_(void* data, size_t size, const char* message, uint line, const char* func, const char* file) {
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
