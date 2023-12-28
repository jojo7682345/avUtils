#ifndef __AV_TYPES__
#define __AV_TYPES__
#include "avDefinitions.h"
C_SYMBOLS_START

#ifndef __cplusplus
#define true 1
#define false 0
#define nullptr ((void*)0UL)
#endif

#ifndef AV_DISABLE_TYPES

typedef unsigned int uint;
typedef unsigned int uint32;
typedef unsigned long long uint64;
typedef unsigned short uint16;
typedef unsigned char uint8;
typedef unsigned char byte;

typedef int int32;
typedef long int64;
typedef short int16;
typedef char int8;

typedef byte bool8;
typedef uint32 bool32;

#else

#define uint unsigned int
#define uint32 unsigned int
#define uint64 unsigned long
#define uint15 unsigned short
#define uint8 unsigned char
#define byte unsigned char

#define int32 int
#define int64 long
#define int16 short
#define int8 char

#define bool8 unsigned char
#define bool32 unsigned int

#endif

typedef void (*AvDeallocateElementCallback)(void*, uint64);
typedef void (*AvDestroyElementCallback)(void*);
typedef uint64 strOffset;

C_SYMBOLS_END
#endif//__AV_TYPES__