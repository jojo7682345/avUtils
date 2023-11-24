#include <AvUtils/avString.h>
#include <AvUtils/avMemory.h>
#include <AvUtils/avMath.h>
#include <AvUtils/dataStructures/avDynamicArray.h>

#include <string.h>
#include <stdio.h>
#include <stdarg.h>

typedef struct StringDebugContext_T* StringDebugContext;
typedef struct StringDebugContext_T {
	StringDebugContext prev;
	AvDynamicArray allocatedMemory;
} StringDebugContext_T;
static StringDebugContext debugContext;

uint64 avStringLength(const char* str) {
	uint64 length = 0;
	while (str[length++]) {}
	return length-1;
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
			if (str.chrs[i-1 + j] != find.chrs[j]) {
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

void avStringReplaceChar(AvString str, char original, char replacement) {
	for (uint64 i = 0; i < str.len; i++) {
		if (str.chrs[i] == original) {
			str.chrs[i] = replacement;
		}
	}
}

uint64 avStringReplace(AvString str, AvString sequence, AvString replacement, AvAllocatedString* result) {

	uint64 count = avStringFindCount(str, sequence);

	if (count == 0) {
		if (result) {
			avAllocatedStringCreate(str, result);
		}
		return 0;
	}
	if (result == NULL) {
		return count;
	}

	uint64 remainingLength = str.len - count * sequence.len;
	uint64 newLength = remainingLength + count * replacement.len;

	avAllocatedStringCreate(AV_STRL(nullptr, newLength), result);

	uint64 writeIndex = 0;
	uint64 readIndex = 0;
	uint64 countReplaced = 0;

	while (countReplaced != count) {
		strOffset offset = avStringFindFirstOccuranceOf(str, sequence);
		for (uint64 i = 0; i < offset; i++) {
			result->memory->data[writeIndex++] = str.chrs[i];
			readIndex++;
		}
		for (uint64 i = 0; i < replacement.len; i++) {
			result->memory->data[writeIndex++] = replacement.chrs[i];
		}
		readIndex += sequence.len;
		countReplaced++;
		str.chrs += offset + sequence.len;
	}
	for(int i = 0; i < str.len-readIndex; i++){
		result->memory->data[writeIndex++] = str.chrs[i];
	}

	return count;
}

void avStringJoin(AvString strA, AvString strB, AvAllocatedString* result){

	uint64 len = strA.len + strB.len;

	avAllocatedStringCreate(AV_STRL(nullptr, len), result);
	avStringMemoryStore(strA, 0, AV_STRING_FULL_LENGTH, result->memory);
	avStringMemoryStore(strB, strA.len, AV_STRING_FULL_LENGTH, result->memory);
}

void avStringMemoryCreatePersistent(AvPersistentStringMemory* memory){
	(*memory) = avCallocate(1, sizeof(AvStringMemory), "allocating persistent Memory");
	(*memory)->persistentlyAllocated = true;
}

void avStringMemoryAllocate(uint64 size, AvStringMemory* memory) {
#ifndef NDEBUG
	if (debugContext) {
		memory->allocationIndex = avDynamicArrayAdd(&memory, debugContext->allocatedMemory);
	}
#endif

	memory->data = avCallocate(size+1, 1, "allocating string memory");
	memory->size = size;
	memory->referenceCount = 0;
}

void avStringMemoryStore(AvString str, uint64 offset, uint64 len, AvStringMemory* memory) {
	if(offset >= memory->size){
		return;
	}
	if(len == AV_STRING_FULL_LENGTH){
		len = str.len;
	}
	len = AV_MIN(memory->size, len + offset) - offset;

	memcpy(memory->data + offset, str.chrs, len);
}

void avStringMemoryStoreCharArraysAV_(AvAllocatedString* result, ...){
	
	AvDynamicArray arr;
	avDynamicArrayCreate(0, sizeof(const char*), &arr);

	
	
	va_list list;
	va_start(list, result);
	const char* arg = NULL;
	while(arg = va_arg(list, const char*)){
		avDynamicArrayAdd(&arg, arr);
	}
	va_end(list);

	uint32 count = avDynamicArrayGetSize(arr);
	const char** strs = avCallocate(count, sizeof(const char*), "allocating strings");
	avDynamicArrayReadRange(strs, count, 0, sizeof(const char*), 0, arr);
	avDynamicArrayDestroy(arr);

	avStringMemoryStoreCharArrays(result, count, strs);

	avFree(strs);
}

void avStringMemoryStoreCharArrays(AvAllocatedString* result, uint32 count, const char* strs[]){

	if(count == 0){
		result->str = (AvString){0};
		result->memory = NULL;
		return;
	}

	uint64 size = 0;
	for(uint32 i = 0; i < count; i++){
		size += avStringLength(strs[i]);
	}

	avAllocatedStringCreate(AV_STRL(nullptr, size), result);
	uint64 offset = 0;
	for(uint i = 0; i < count; i++){
		uint64 size = avStringLength(strs[i]);
		avStringMemoryStore(AV_STRL((char*)strs[i], size), offset, size, result->memory);
		offset += size;
	}
}

void avStringMemoryCreateString(uint64 offset, uint64 len, AvAllocatedString* allocatedString, AvStringMemory* memory) {
	if (offset >= memory->size) {
		allocatedString->str = AV_STRL(nullptr, 0);
		allocatedString->memory = nullptr;
		return;
	}
	if (len == AV_STRING_FULL_LENGTH) {
		len = memory->size;
	}
	len = AV_MIN(memory->size, len + offset) - offset;
	memory->referenceCount++;
	allocatedString->memory = memory;
	allocatedString->str.len = len;
	allocatedString->str.chrs = memory->data + offset;
}


void avStringClone(uint64 offset, uint64 len, AvString str, AvAllocatedString* allocatedString) {
	if(offset >= str.len){
		return;
	}
	if(len==AV_STRING_FULL_LENGTH){
		len = str.len;
	}
	len = AV_MIN(str.len, len + offset) - offset;
	str.chrs += offset;
	str.len = len;
	avAllocatedStringCreate(str, allocatedString);
}

void avAllocatedStringCreate(AvString str, AvAllocatedString* allocatedString) {
	
	AvStringMemory* memory = avCallocate(1, sizeof(AvStringMemory), "allocating string memory handle");
	avStringMemoryAllocate(str.len, memory);
	if (str.chrs) {
		avStringMemoryStore(str, 0, AV_STRING_FULL_LENGTH, memory);
	}
	avStringMemoryCreateString(0, str.len, allocatedString, memory);

}

void avAllocatedStringDestroy(AvAllocatedString* str) {
	if(str==NULL){
		return;
	}
	if (str->memory && --str->memory->referenceCount == 0) {
		avStringMemoryFree(str->memory);
	}
	str->str = (AvString){ 0 };
	str->memory = nullptr;
}

void avStringMemoryFree(AvStringMemory* memory) {
	
	avFree(memory->data);
	memory->data = nullptr;
	memory->referenceCount = 0;
	memory->size = 0;
#ifndef NDEBUG
	if (debugContext) {
		avDynamicArrayRemove(memory->allocationIndex, debugContext->allocatedMemory);
		for (uint32 i = memory->allocationIndex; i < avDynamicArrayGetSize(debugContext->allocatedMemory); i++) {
			AvStringMemory* tmp;
			avDynamicArrayRead(&tmp, i, debugContext->allocatedMemory);
			tmp->allocationIndex--;
			avDynamicArrayWrite(&tmp, i, debugContext->allocatedMemory);
		}	
	}
#endif
	if(memory->persistentlyAllocated){
		avFree(memory);
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



void avStringDebugContextStart_() {
	
	StringDebugContext context = avCallocate(1, sizeof(StringDebugContext_T), "allocating string debug context");
	context->prev = debugContext;
	avDynamicArrayCreate(0, sizeof(AvStringMemory*), &context->allocatedMemory);
	debugContext = context;

}

void avStringDebugContextEnd_() {
	uint unfreedMemory = avDynamicArrayGetSize(debugContext->allocatedMemory);
	for (uint i = 0; i < unfreedMemory; i++) {
		AvStringMemory* stringMemory;
		avDynamicArrayRead(&stringMemory, 0, debugContext->allocatedMemory);

		//TODO: better log
		printf("allocated string memory containing \"%.*s\" has not been freed: %i remaining references\n", stringMemory->size, stringMemory->data, stringMemory->referenceCount);
		avStringMemoryFree(stringMemory);
	}
	avDynamicArrayDestroy(debugContext->allocatedMemory);


	StringDebugContext nextContext = debugContext->prev;
	avFree(debugContext);
	debugContext = nextContext;
}




