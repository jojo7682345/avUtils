#ifndef __AV_TYPES__
#define __AV_TYPES__

#include <stdint.h>

#ifndef AV_DISABLE_TYPES

typedef unsigned int uint;
typedef unsigned int uint32;
typedef unsigned long uint64;
typedef unsigned short uint16;
typedef unsigned char uint8;
typedef unsigned char byte;

#else

#define uint unsigned int
#define uint32 unsigned int
#define uint64 unsigned long
#define uint15 unsigned short
#define uint8 unsigned char
#define byte unsigned char

#endif

#endif //__AV_TYPES__