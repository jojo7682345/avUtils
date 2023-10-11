#include <avMemory.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

byte* avAllocate_(size_t size, const char* message, uint line, const char* func, const char* file) {
	byte* data = (byte*)malloc(size);

	if (!data) {
		fprintf(stderr, "malloc returned null: %s\n", message);
		exit(-1);
		return NULL;
	}

	return data;
}

byte* avCallocate_(size_t count, size_t size, const char* message, uint line, const char* func, const char* file) {
	byte* data = (byte*)calloc(count, size);

	if (!data) {
		fprintf(stderr, "calloc returned null: %s\n", message);
		exit(-1);
		return NULL;
	}

	return data;
}

byte* avReallocate_(byte* data, size_t size, const char* message, uint line, const char* func, const char* file) {
	byte* newPtr = (byte*)realloc(data, size);

	if (!newPtr) {
		fprintf(stderr, "realloc returned null: %s\n", message);
		exit(-1);
		return NULL;
	}

	return newPtr;
}

void avFree_(byte* data, uint line, const char* func, const char* file) {


	free(data);
}
