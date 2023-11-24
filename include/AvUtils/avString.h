#ifndef __AV_STRING__
#define __AV_STRING__
#include "avDefinitions.h"
C_SYMBOLS_START

#include "avTypes.h"

#define AV_STRING_FULL_LENGTH 0
#define AV_STRING_NULL ((uint64)-1)
#define AV_STRING_PRINTF_CODE "%.*s"

typedef struct AvStringMemoryProperties {
	bool8 heapAllocated;
	uint32 contextAllocationIndex; // for debugging purposes
} AvStringMemoryProperties;
typedef struct AvStringMemory {
	char* data; // the characters
	uint64 capacity; // the amount of characters allocated

	uint64 referenceCount;
	void** references;
	AvStringMemoryProperties properties;
} AvStringMemory;
typedef AvStringMemory* AvStringHeapMemory;
typedef AvStringMemory* AvStringMemoryRef;

/// @brief a length terminated string
typedef struct AvString {
	const char* const chrs; // the pointer to the string
	const uint64 len; // the length of the string
	AvStringMemory* const memory; // reference to a backing string memory, if it is a nullptr this string cannot be mutated
} AvString;
typedef AvString* AvStringRef;

/// @brief a constant denoting the string has no backing memory, and therefore cannot be allocated
#define AV_STRING_CONST ((AvStringMemory*)0)

/// @brief create a string from a existing const char* and a length
#define AV_STR(str, length) ((AvString){.chrs=str, .len=length, .memory=AV_STRING_CONST}) 
/// @brief create a string from a exisitng const char*
#define AV_CSTR(str) AV_STR(str, avCStringLength(str)) 

void avStringCopy(AvStringRef dst, AvString src);
void avStringMove(AvStringRef dst, AvStringRef src);

void avStringFromMemory(AvStringRef dst, uint64 offset, uint64 length, AvStringMemoryRef memory);

/// @brief removes a reference from the string memory, freeing it 
/// @param str 
void avStringFree(AvStringRef str);

/// @brief calculate the length of a null terminated character array
/// @param str a null terminated character array
/// @return the length of the string excluding the termination character
uint64 avCStringLength(const char* str);

/// @brief allocate the handle for the string memory on the heap
/// @param capacity the amound of characters t
/// @param memory pointer to a handle to the string memory
void avStringMemoryHeapAllocate(uint64 capacity, AvStringHeapMemory* memory);

/// @brief allocates the data for a string 
/// @param capacity the amound of characters to allocate
/// @param memory reference to the string memory
void avStringMemoryAllocate(uint64 capacity, AvStringMemoryRef memory);


/// @brief resizes the string memory
/// @param capacity the new capacity
/// @param memory reference to the string memory
void avStringMemoryResize(uint64 capacity, AvStringMemoryRef memory);

/// @brief frees the data allocated for the string memory
/// @param memory reference to the string memory
void avStringMemoryFree(AvStringMemoryRef memory);

/// @brief store the string into the memory
/// @param str the data to be stored
/// @param offset the offset into the memory
/// @param length the length stored in the memory
/// @param memory reference to the string memory
void avStringMemoryStore(AvString str, uint64 offset, uint64 length, AvStringMemoryRef memory);

#ifndef NDEBUG
#define avStringDebugContextStart avStringDebugContextStart_();
#define avStringDebugContextEnd avStringDebugContextEnd_()
#else
#define avStringDebugContextStart() (void)
#define avStringDebugContextEnd() (void)
#endif

void avStringDebugContextStart_();
void avStringDebugContextEnd_();

strOffset avStringFindLastOccuranceOfChar(AvString str, char chr);
strOffset avStringFindFirstOccranceOfChar(AvString str, char chr);
uint64 avStringFindCharCount(AvString str, char chr);

strOffset avStringFindLastOccuranceOf(AvString str, AvString find);
strOffset avStringFindFirstOccuranceOf(AvString str, AvString find);
uint64 avStringFindCount(AvString str, AvString find);


void avStringReplaceChar(AvStringRef str, char original, char replacement);
uint64 avStringReplace(AvStringRef str, AvString sequence, AvString replacement);

void avStringJoin(AvStringRef dst, AvString strA, AvString strB);

#define avStringMemoryStoreCharArraysVA(result, ...) avStringMemoryStoreCharArraysVA_(result, __VA_ARGS__, NULL);
void avStringMemoryStoreCharArraysVA_(AvStringMemoryRef result, ...);
void avStringMemoryStoreCharArrays(AvStringMemoryRef result, uint32 count, const char* strs[]);

void avStringPrint(AvString str);
void avStringPrintLn(AvString str);
void avStringPrintln(AvString str);

C_SYMBOLS_END
#endif//__AV_STRING__
