#ifndef __AV_TYPES__
#define __AV_TYPES__

#ifndef __cplusplus
#define true 1
#define false 0
#define nullptr ((void*)0UL)
#endif

#ifndef AV_DISABLE_TYPES

typedef unsigned int uint;
typedef unsigned int uint32;
typedef unsigned long uint64;
typedef unsigned short uint16;
typedef unsigned char uint8;
typedef unsigned char byte;

typedef byte bool8;
typedef uint32 bool32;

#else

#define uint unsigned int
#define uint32 unsigned int
#define uint64 unsigned long
#define uint15 unsigned short
#define uint8 unsigned char
#define byte unsigned char

#define bool8 unsigned char
#define bool32 unsigned int

#endif

#endif //__AV_TYPES__