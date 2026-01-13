#ifndef __AV_TYPES__
#define __AV_TYPES__

#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif

#ifndef NULL
#define NULL 0
#endif

#ifndef nullptr 
#define nullptr (void*){ 0 }
#endif

#ifndef AV_UTILS_DISABLE_TYPEDEFS


#if !(defined(__INT64_TYPE__) || defined(__INT32_TYPE__) || defined(__INT16__TYPE__) || defined(__INT8_TYPE__))
#include <stdint.h>
#endif

#ifdef __INT64_TYPE__
typedef __UINT64_TYPE__ uint64;
typedef __INT64_TYPE__ int64;
typedef __UINT64_TYPE__ bool64;
typedef __UINT64_TYPE__ u64;
typedef __INT64_TYPE__ i64;
#else
typedef uint64_t uint64;
typedef int64_t int64;
typedef uint64_t bool64;
typedef uint64_t u64;
typedef int64_t i64;
#endif

#ifdef __INT32_TYPE__
typedef __UINT32_TYPE__ uint32;
typedef __INT32_TYPE__ int32;
typedef __UINT32_TYPE__ bool32;
typedef __UINT32_TYPE__ uint;
typedef __UINT32_TYPE__ u32;
typedef __INT32_TYPE__ i32;
#else
typedef uint32_t uint32;
typedef int32_t int32;
typedef uint32_t bool32;
typedef uint32_t uint;
typedef uint32_t u32;
typedef int32_t i32;
#endif

#ifdef __INT16_TYPE__
typedef __UINT16_TYPE__ uint16;
typedef __INT16_TYPE__ int16;
typedef __UINT16_TYPE__ bool16;
typedef __UINT16_TYPE__ u16;
typedef __INT16_TYPE__ i16;
#else
typedef uint16_t uint16;
typedef int16_t int16;
typedef uint16_t bool16;
typedef uint16_t u16;
typedef int16_t i16;
#endif

#ifdef __INT8_TYPE__
typedef __UINT8_TYPE__ uint8;
typedef __INT8_TYPE__ int8;
typedef __UINT8_TYPE__ byte;
typedef __INT8_TYPE__ sbyte;
typedef __UINT8_TYPE__ bool8;
typedef __UINT8_TYPE__ u8;
typedef __INT8_TYPE__ i8;
#else
typedef uint8_t uint8;
typedef int8_t int8;
typedef uint8_t byte;
typedef int8_t sbyte;
typedef uint8_t bool8;
typedef uint8_t u8;
typedef int8_t i8;
#endif


#else //AV_UTILS_DISABLE_TYPEDEFS

#ifdef __INT64_TYPE__
#define uint64 __UINT64_TYPE__
#define int64 __INT64_TYPE__
#define bool64 __UINT64_TYPE__
#define u64 __UINT64_TYPE__
#define i64 __INT64_TYPE__
#endif

#ifdef __INT32_TYPE__
#define uint32 __UINT32_TYPE__
#define int32 __INT32_TYPE__
#define bool32 __UINT32_TYPE__
#define u32 __UINT32_TYPE__
#define i32 __INT32_TYPE__
#endif

#ifdef __INT16_TYPE__
#define uint16 __UINT16_TYPE__
#define int16 __INT16_TYPE__
#define bool16 __UINT16_TYPE__
#define u16 __UINT16_TYPE__
#define i16 __INT16_TYPE__
#endif

#ifdef __INT8_TYPE__
#define uint8 __UINT8_TYPE__
#define int8 __INT8_TYPE__
#define byte __UINT8_TYPE__
#define sbyte __INT8_TYPE__
#define bool8 __UINT8_TYPE__
#define u8 __UINT8_TYPE__
#define i8 __INT8_TYPE__
#endif

#endif//AV_UTILS_DISABLE_TYPEDEFS

#endif//__AV_TYPES__