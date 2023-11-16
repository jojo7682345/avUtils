#ifndef __AV_STRING__
#define __AV_STRING__

#include "avTypes.h"

#define AV_STRING_NULL ((uint64)-1)

typedef struct AvString {
	char* chrs;
	uint64 len;
} AvString;

typedef struct AvStringMemory {
	char* data;
	uint64 size;
	uint32 referenceCount;
#ifndef NDEBUG
	uint32 allocationIndex;
#endif
}AvStringMemory;

typedef struct AvAllocatedString {
	AvString str;
	AvStringMemory* memory;
} AvAllocatedString;

#define AV_STRL(str, length) (AvString){ .chrs=str, .len=length }
#define AV_STR(str) AV_STRL(str,avStringLength(str)-1)
#define AV_STR_ARR(arr) AV_STRL(arr,sizeof(arr)/sizeof(char))

uint64 avStringLength(const char* str);

strOffset avStringFindLastOccuranceOfChar(AvString str, char chr);
strOffset avStringFindFirstOccranceOfChar(AvString str, char chr);
uint64 avStringFindCharCount(AvString str, char chr);

strOffset avStringFindLastOccuranceOf(AvString str, AvString find);
strOffset avStringFindFirstOccuranceOf(AvString str, AvString find);
uint64 avStringFindCount(AvString str, AvString find);


void avStringReplaceChar(AvString str, char original, char replacement);

uint64 avStringReplace(AvString str, AvString sequence, AvString replacement, AvAllocatedString* result);

void avStringMemoryAllocate(uint64 size, AvStringMemory* memory);
void avAllocatedStringCreate(AvString str, AvAllocatedString* allocatedString);
void avAllocatedStringDestroy(AvAllocatedString* str);
void avStringMemoryFree(AvStringMemory* memory);

void AvStringPrint(AvString str);

#ifndef NDEBUG
#define avStringDebugContextStart avStringDebugContextStart_()
#define avStringDebugContextEnd avStringDebugContextEnd_()
#else
#define avStringDebugContextStart() (void)
#define avStringDebugContextEnd() (void)
#endif

void avStringDebugContextStart_();
void avStringDebugContextEnd_();


#endif//__AV_STRING__
