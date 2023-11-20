#ifndef __AV_STRING__
#define __AV_STRING__

#include "avTypes.h"

#define AV_STRING_NULL ((uint64)-1)
#define AV_STRING_FULL_LENGTH 0
#define AV_STRING_PRINTF_CODE "%.*s"

typedef struct AvString {
	uint64 len;
	char* chrs;
} AvString;

typedef struct AvStringMemory {
	char* data;
	uint64 size;
	uint32 referenceCount;
	bool32 persistentlyAllocated;
#ifndef NDEBUG
	uint32 allocationIndex;
#endif
}AvStringMemory;
typedef AvStringMemory* AvPersistentStringMemory;

typedef struct AvAllocatedString {
	AvString str;
	AvStringMemory* memory;
} AvAllocatedString;

typedef struct AvConstString {
	uint64 len;
	const char* chrs;
} AvConstString;

#define AV_STRL(str, length) (AvString){ .chrs=str, .len=length }
#define AV_STR(str) AV_STRL(str,avStringLength(str))
#define AV_STR_ARR(arr) AV_STRL(arr,sizeof(arr)/sizeof(char))

#define AV_CSTRL(str, length) (AvConstString){ .chrs=str, .len=length }
#define AV_CSTR(str) AV_CSTRL(str, avStringLength(str))
#define AV_CSTR_ARR(arr) AV_CSTRL(arr, sizeof(arr)/sizeof(char))

uint64 avStringLength(const char* str);

strOffset avStringFindLastOccuranceOfChar(AvString str, char chr);
strOffset avStringFindFirstOccranceOfChar(AvString str, char chr);
uint64 avStringFindCharCount(AvString str, char chr);

strOffset avStringFindLastOccuranceOf(AvString str, AvString find);
strOffset avStringFindFirstOccuranceOf(AvString str, AvString find);
uint64 avStringFindCount(AvString str, AvString find);


void avStringReplaceChar(AvString str, char original, char replacement);

uint64 avStringReplace(AvString str, AvString sequence, AvString replacement, AvAllocatedString* result);
void avStringJoin(AvString strA, AvString strB, AvAllocatedString* result);

#define avStringMemoryStoreCharArraysVA(result, ...) avStringMemoryStoreCharArraysVA_(result, __VA_ARGS__, NULL);
void avStringMemoryStoreCharArraysVA_(AvAllocatedString* result, ...);
void avStringMemoryStoreCharArrays(AvAllocatedString* result, uint32 count, const char* strs[]);

void avStringMemoryCreatePersistent(AvPersistentStringMemory* memory);
void avStringMemoryAllocate(uint64 size, AvStringMemory* memory);
void avStringMemoryStore(AvString str, uint64 offset, uint64 len, AvStringMemory* memory);
void avStringMemoryCreateString(uint64 offset, uint64 len, AvAllocatedString* allocatedString, AvStringMemory* memory);

void avStringClone(uint64 offset, uint64 len, AvString str, AvAllocatedString* allocatedString);

void avAllocatedStringCreate(AvString str, AvAllocatedString* allocatedString);
void avAllocatedStringDestroy(AvAllocatedString* str);
void avStringMemoryFree(AvStringMemory* memory);


void avStringPrint(AvString str);
void avStringPrintLn(AvString str);
void avStringPrintln(AvString str);

#ifndef NDEBUG
#define avStringDebugContextStart avStringDebugContextStart_();
#define avStringDebugContextEnd avStringDebugContextEnd_()
#else
#define avStringDebugContextStart() (void)
#define avStringDebugContextEnd() (void)
#endif



void avStringDebugContextStart_();
void avStringDebugContextEnd_();


#endif//__AV_STRING__
