#ifndef __AV_TYPES__
#define __AV_TYPES__
#include "avDefinitions.h"
C_SYMBOLS_START

#ifndef __cplusplus
#define true 1
#define false 0
#define nullptr ((void*)0UL)
#endif

#define avUint unsigned int
#define avUint32 unsigned int

#ifdef _WIN32
#define avUint64 unsigned long long
#else
#define avUint64 unsigned long
#endif

#define avUint16 unsigned short
#define avUint8 unsigned char
#define avByte unsigned char

#define avInt32 int
#define avInt64 long
#define avInt16 short
#define avInt8 char

#define avBool8 unsigned char
#define avBool32 unsigned int

#ifndef AV_DISABLE_TYPES

typedef avUint uint;
typedef avUint32 uint32;
typedef avUint64 uint64;
typedef avUint16 uint16;
typedef avUint8 uint8;
typedef avByte byte;

typedef avInt32 int32;
typedef avInt64 int64;
typedef avInt16 int16;
typedef avInt8 int8;

typedef avBool8 bool8;
typedef avBool32 bool32;

#else

#define uint avUint
#define uint32 avUint32
#define uint64 avUint64
#define uint16 avUint16
#define uint8 avUint8
#define byte avByte

#define int32 avInt32
#define int64 avInt64
#define int16 avInt16
#define int8 avInt8

#define bool8 avBool8
#define bool32 avBool32

#endif

typedef void (*AvDeallocateElementCallback)(void*, uint64);
typedef void (*AvDestroyElementCallback)(void*);
typedef uint64 strOffset;

C_SYMBOLS_END
#endif//__AV_TYPES__