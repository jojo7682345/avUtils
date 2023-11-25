#include <AvUtils/avString.h>
#include <AvUtils/avMemory.h>
#include <AvUtils/avMath.h>
#include <AvUtils/dataStructures/avDynamicArray.h>
#include <AvUtils/avLogging.h>

#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#define NULL_TERMINATOR_SIZE 1

typedef struct StringDebugContext_T* StringDebugContext;
typedef struct StringDebugContext_T {
	StringDebugContext prev;
	AvDynamicArray allocatedMemory;
} StringDebugContext_T;
static StringDebugContext debugContext;

uint64 avCStringLength(const char* str) {
	uint64 length = 0;
	while (str[length++]) {} // loop until the termination character
	return length - 1; // do not include the termination character
}

void avStringCopy(AvStringRef dst, AvString src) {
	AvStringHeapMemory memory;
	avStringMemoryHeapAllocate(src.len, &memory);
	avStringMemoryStore(src, 0, AV_STRING_FULL_LENGTH, memory);
	avStringFromMemory(dst, 0, AV_STRING_FULL_LENGTH, memory);
}

void avStringMove(AvStringRef dst, AvStringRef src) {
	avAssert(dst != nullptr, "destination must be a valid reference");
	avAssert(src != nullptr, "source must be a valid reference");

	avStringCopy(dst, *src);
	avStringFree(src);
}

void avStringFromMemory(AvStringRef dst, uint64 offset, uint64 length, AvStringMemoryRef memory) {
	avAssert(memory != 0, "memory must be valid");
	avAssert(dst != 0, "destination must be a valid reference");

	memset(dst, 0, sizeof(AvString)); // empty the string

	if (offset >= memory->capacity) {
		avAssert(offset >= memory->capacity, "offset is greater than capacity resulting in an empty string");
		return;
	}
	if (length == AV_STRING_FULL_LENGTH) {
		length = memory->capacity;
	}
	length = AV_MIN(memory->capacity, length + offset) - offset;

	AvString result = {
		.chrs = memory->data + offset,
		.len = length,
		.memory = memory
	};
	memcpy(dst, &result, sizeof(AvString));

	// update references
	uint32 referenceIndex = memory->referenceCount++;
	memory->references = avReallocate(memory->references, sizeof(AvString) * memory->referenceCount, "reallocating memory references");
	memory->references[referenceIndex] = dst;
}

static uint32 stringMemoryFindReference(AvStringRef ref, AvStringMemoryRef memory) {
	avAssert(memory!=nullptr, "memory must be a valid reference");
	avAssert(ref!=nullptr, "ref must be a valid reference");
	for (uint32 i = 0; i < memory->referenceCount; i++) {
		if (memory->references[i] == ref) {
			return i;
		}
	}
	return (uint32)-1;
}

static void stringMemoryRemoveReference(uint32 refIndex, AvStringMemoryRef memory) {
	avAssert(memory != nullptr, "memory must be a valid reference");
	avAssert(refIndex != (uint32)-1, "refIndex must be a valid index");
	avAssert(refIndex < memory->referenceCount, "refIndex is invalide");
	uint32 remainingReferences = memory->referenceCount - refIndex - 1;
	memcpy(memory->references + refIndex, memory->references + refIndex + 1, sizeof(AvStringRef) * remainingReferences);
	avReallocate(memory->references, sizeof(AvStringRef) * (memory->referenceCount - 1), "reallocating memory references");
	memory->referenceCount--;

	if (memory->referenceCount == 0) {
		avStringMemoryFree(memory);
	}
}

void avStringFree(AvStringRef str) {

	avAssert(str != nullptr, "string must be a valid reference");
	if (str->memory == nullptr) {
		return;
	}
	AvStringMemoryRef memory = str->memory;
	avAssert(memory->referenceCount > 0, "string memory references are corrupt");
	
	uint32 refIndex = stringMemoryFindReference(str, memory);
	stringMemoryRemoveReference(refIndex, memory);
}

void avStringMemoryHeapAllocate(uint64 capacity, AvStringHeapMemory* memory) {
	avAssert(memory != nullptr, "invalid heap memory pointer");
	(*memory) = avCallocate(1, sizeof(AvStringMemory), "allocating string memory on heap");
	avStringMemoryAllocate(capacity, *memory);
	(*memory)->properties.heapAllocated = true;
}

void avStringMemoryAllocate(uint64 capacity, AvStringMemoryRef memory) {
	avAssert(memory != nullptr, "invalid memory reference");
	avAssert(memory->data == nullptr, "string memory already allocated");
	avAssert(memory->capacity == 0, "string memory already allocated");
	avAssert(memory->referenceCount == 0, "string memory already allocated");
	avAssert(memory->references == nullptr, "string memory already allocated");
	avAssert(capacity != 0, "capacity must be >1");

	memory->capacity = capacity;
	memory->data = avCallocate(capacity + NULL_TERMINATOR_SIZE, 1, "allocating string data");
	memory->referenceCount = 0;
	memory->properties.heapAllocated = false;

#ifndef NDEBUG
	if (debugContext) {
		memory->properties.contextAllocationIndex = avDynamicArrayAdd(&memory, debugContext->allocatedMemory);
	}
#endif

}

void avStringMemoryResize(uint64 capacity, AvStringMemoryRef memory) {
	avAssert(memory!=nullptr, "invalid memory reference");
	avAssert(capacity, "resize to 0 size is not allowed");
	avAssert(memory->data != nullptr, "resizing unallocated memory is not allowed");
	avAssert(memory->capacity, "resizing unallocated memory is not allowed");

	char* prevData = memory->data;
	uint64 prevCapacity = memory->capacity;
	memory->data = avReallocate(memory->data, capacity + NULL_TERMINATOR_SIZE, "reallocating string data");
	memory->capacity = capacity;

	// update references to new data
	for (uint32 i = 0; i < memory->referenceCount; i++) {
		AvStringRef str = memory->references[i];
		avAssert(str != nullptr, "references are corrupted");
		avAssert(str->memory == memory, "string must be allocated from this memory");

		uint64 offset = str->chrs - prevData;
		uint64 length = AV_MIN(str->len + offset, capacity) - offset;

		if (offset >= capacity || length == 0) {
			stringMemoryRemoveReference(i, memory);
			memset(str, 0, sizeof(AvString));
			i--;
			continue;
		}

		AvString tmpStr = {
			.chrs = memory->data + offset,
			.len = length,
			.memory = memory
		};
		memcpy(str, &tmpStr, sizeof(AvString));
	}

}

void avStringMemoryFree(AvStringMemoryRef memory) {
	avAssert(memory != nullptr, "invalid memory reference");
	avAssert(memory->referenceCount == 0, "freeing string memory still in use");

#ifndef NDEBUG
	if (debugContext) {
		avDynamicArrayRemove(memory->properties.contextAllocationIndex, debugContext->allocatedMemory);
		for (uint32 i = memory->properties.contextAllocationIndex; i < avDynamicArrayGetSize(debugContext->allocatedMemory); i++) {
			AvStringMemoryRef tmp;
			avDynamicArrayRead(&tmp, i, debugContext->allocatedMemory);
			tmp->properties.contextAllocationIndex--;
			avDynamicArrayWrite(&tmp, i, debugContext->allocatedMemory);
		}
	}
#endif

	if (memory->data) {
		avFree(memory->data);
		memory->data = nullptr;
	}

	if (memory->properties.heapAllocated) {
		avFree(memory);
	}
}

void avStringMemoryStore(AvString str, uint64 offset, uint64 length, AvStringMemoryRef memory) {
	avAssert(memory != nullptr, "memory must be a valid reference");
	avAssert(memory->data != nullptr, "memory has not been allocated");
	avAssert(memory->capacity != 0, "memory has not been allocated");
	if (offset >= memory->capacity) {
		avAssert(offset >= memory->capacity, "offset is greater than capacity resulting in an empty string");
		return;
	}
	if (str.len == 0) {
		return;
	}
	if (length == AV_STRING_FULL_LENGTH) {
		length = str.len;
	}
	length = AV_MIN(memory->capacity, length + offset) - offset; // cap length to memory capacity
	length = AV_MIN(length, str.len);
	avAssert(str.chrs != nullptr, "string must be valid if str.len > 0");
	memcpy(memory->data + offset, str.chrs, length);
}

void avStringDebugContextStart_() {

	StringDebugContext context = avCallocate(1, sizeof(StringDebugContext_T), "allocating string debug context");
	context->prev = debugContext;
	avDynamicArrayCreate(0, sizeof(AvStringMemoryRef), &context->allocatedMemory);
	debugContext = context;

}

void avStringDebugContextEnd_() {
	uint unfreedMemory = avDynamicArrayGetSize(debugContext->allocatedMemory);
	for (uint i = 0; i < unfreedMemory; i++) {
		AvStringMemoryRef stringMemory;
		avDynamicArrayRead(&stringMemory, 0, debugContext->allocatedMemory);

		//TODO: better log
		printf("allocated string memory containing \"%.*s\" has not been freed: %i remaining references\n", stringMemory->capacity, stringMemory->data, stringMemory->referenceCount);
		avStringMemoryFree(stringMemory);
	}
	avDynamicArrayDestroy(debugContext->allocatedMemory);


	StringDebugContext nextContext = debugContext->prev;
	avFree(debugContext);
	debugContext = nextContext;
}

strOffset avStringFindLastOccuranceOfChar(AvString str, char chr) {
	for (uint64 i = str.len; i > 0; i--) {
		if (str.chrs[i - 1] == chr) {
			return i - 1;
		}
	}
	return AV_STRING_NULL;
}

strOffset avStringFindFirstOccranceOfChar(AvString str, char chr) {
	for (uint64 i = 0; i < str.len; i++) {
		if (str.chrs[i] == chr) {
			return i;
		}
	}
	return AV_STRING_NULL;
}

uint64 avStringFindCharCount(AvString str, char chr) {
	uint64 count = 0;
	for (uint64 i = 0; i < str.len; i++) {
		if (str.chrs[i] == chr) {
			count++;
		}
	}
	return count;
}

strOffset avStringFindLastOccuranceOf(AvString str, AvString find) {
	if (str.len < find.len) {
		return AV_STRING_NULL;
	}

	for (uint64 i = str.len - find.len; i > 0; i--) {
		uint64 j = 0;
		for (; j < find.len; j++) {
			if (str.chrs[i - 1 + j] != find.chrs[j]) {
				break;
			}
		}
		if (j == find.len) {
			return i;
		}
	}
	return AV_STRING_NULL;
}

strOffset avStringFindFirstOccuranceOf(AvString str, AvString find) {
	uint64 strOffset = 0;
	if (str.len < find.len) {
		return AV_STRING_NULL;
	}

	for (uint64 i = 0; i < str.len; i++) {
		if (str.chrs[i] == find.chrs[strOffset]) {
			strOffset++;
			if (strOffset == find.len) {
				return i;
			}
		} else {
			strOffset = 0;
		}
	}
	return AV_STRING_NULL;
}

uint64 avStringFindCount(AvString str, AvString find) {
	uint64 strOffset = 0;
	if (str.len < find.len) {
		return 0;
	}
	uint64 count = 0;
	for (uint64 i = 0; i < str.len; i++) {
		if (str.chrs[i] == find.chrs[strOffset]) {
			strOffset++;
			if (strOffset == find.len) {
				count++;
				strOffset = 0;
			}
		} else {
			strOffset = 0;
		}
	}
	return count;
}

void avStringReplaceChar(AvStringRef str, char original, char replacement) {
	if (str->memory == nullptr) {
		avStringCopy(str, *str);
	}

	for (uint64 i = 0; i < str->len; i++) {
		if (str->memory->data[i] == original) {
			str->memory->data[i] = replacement;
		}
	}
}

uint64 avStringReplace(AvStringRef str, AvString sequence, AvString replacement) {


	uint64 count = avStringFindCount(*str, sequence);

	if (count == 0) {
		return 0;
	}

	if (str->memory == nullptr) {
		avStringCopy(str, *str);
	}

	uint64 remainingLength = str->len - count * sequence.len;
	uint64 newLength = remainingLength + count * replacement.len;

	avStringMemoryResize(newLength, str->memory);
	AvString readStr;
	avStringCopy(&readStr, *str);

	uint64 writeIndex = 0;
	uint64 readIndex = 0;
	uint64 countReplaced = 0;

	while(countReplaced != count){
		AvString remainingStr = AV_STR(
			readStr.chrs + readIndex,
			readStr.len - readIndex - 1
		);
		strOffset offset = avStringFindFirstOccuranceOf(
			remainingStr, 
			sequence
		);
		avStringMemoryStore(remainingStr, readIndex, offset, str->memory);
		writeIndex += offset;
		avStringMemoryStore(replacement, writeIndex, replacement.len, str->memory);
		readIndex += sequence.len;
		countReplaced++;
	}
	AvString remainingStr = AV_STR(
		readStr.chrs + readIndex,
		readStr.len - readIndex - 1
	);
	avStringMemoryStore(remainingStr, readIndex, remainingStr.len, str->memory);
	avStringFree(&readStr);
	return count;
}

void avStringJoin(AvStringRef dst, AvString strA, AvString strB){
	avAssert(dst != nullptr, "destination must be a valid reference");

	uint64 len = strA.len + strB.len;
	avStringMemoryHeapAllocate(len, (AvStringMemoryRef*)&dst->memory);
	avStringMemoryStore(strA, 0, AV_STRING_FULL_LENGTH, dst->memory);
	avStringMemoryStore(strB, strA.len, AV_STRING_FULL_LENGTH, dst->memory);
}

void avStringMemoryStoreCharArraysVA_(AvStringMemoryRef memory, ...) {

	AvDynamicArray arr;
	avDynamicArrayCreate(0, sizeof(const char*), &arr);



	va_list list;
	va_start(list, memory);
	const char* arg = NULL;
	while(arg = va_arg(list, const char*)){
		avDynamicArrayAdd(&arg, arr);
	}
	va_end(list);

	uint32 count = avDynamicArrayGetSize(arr);
	const char** strs = avCallocate(count, sizeof(const char*), "allocating strings");
	avDynamicArrayReadRange(strs, count, 0, sizeof(const char*), 0, arr);
	avDynamicArrayDestroy(arr);

	avStringMemoryStoreCharArrays(memory, count, strs);

	avFree(strs);
}

void avStringMemoryStoreCharArrays(AvStringMemoryRef memory, uint32 count, const char* strs[]){

	uint64 offset = 0;
	for(uint i = 0; i < count; i++){
		uint64 size = avCStringLength(strs[i]);
		avStringMemoryStore(AV_STR(strs[i], size), offset, size, memory);
		offset += size;
	}
}

void avStringPrint(AvString str) {
	printf("%.*s", str.len, str.chrs);
}

void avStringPrintLn(AvString str) {
	avStringPrint(str);
	printf("\n");
}

void avStringPrintln(AvString str){
	avStringPrintLn(str);
}





