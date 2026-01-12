#ifndef __HANDLE__
#define __HANDLE__

#include <avTypes.h>

typedef uint32 avHandle;

#define AvHandle avHandle __attribute((cleanup(avHandleOutOfScope)))
#define AV_NULL_HANDLE ((avHandle){0})
#define AV_INVALID_HANDLE ((avHandle){-1})

#ifndef NDEBUG
#define AV_DEBUG_LOCATION_PARAMS uint32 debugLine, const char* debugFile, const char* debugFunc 
#define AV_DEBUG_LOCATION_ARGS __LINE__, __FILE__, __func__
#define AV_DEBUG_LOCATION_PASS debugLine, debugFile, debugFunc
#else
#define AV_DEBUG_LOCATION_PARAMS __attribute__((unused)) int debugRemnant
#define AV_DEBUG_LOCATION_ARGS 0
#define AV_DEBUG_LOCATION_PASS debugRemnant
#endif

#define avAllocate(size, AV_DEBUG_LOCATION_PASS, parent) avAllocate_(size, AV_DEBUG_LOCATION_ARGS, parent)
avHandle avAllocate_(uint32 size, AV_DEBUG_LOCATION_PARAMS, avHandle parent);

void avFree(avHandle handle);

#define avClone(handle, AV_DEBUG_LOCATION_PASS, parent) avClone_(size, AV_DEBUG_LOCATION_ARGS, parent)
avHandle avClone_(avHandle handle, AV_DEBUG_LOCATION_PARAMS, avHandle parent);

bool8 avHandleGetPtr(avHandle handle, void** ptr, uint32* size);

#endif//__HANDLE__


//#define AvList AvHandle // Free datatype for list of handles
// #define AvString AvString_t __attribute((cleanup(avStringOutofScope)))

// typedef struct AvString_t {
//     union {
//         struct {
//             const char* const chrs; 
//             avHandle memory;
//             uint32 lLength;
//         };
//         struct {
//             const char str[14];
// 			const char nullTerminator;
// 			uint8 sLength; // msb is used to determine if it is stack allocated (max 14 length)
//         };
//     };
// } AvString_t;