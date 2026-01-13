#ifndef __AV_BITFIELD__
#define __AV_BITFIELD__
#include "../avDefinitions.h"
C_SYMBOLS_START

#include "../avTypes.h"

typedef struct AvBitField {
    const uint32 bitCount;
    uint8* const bits;
} AvBitField;

void avBitFieldCreate(uint32 bitCount, AvBitField* bitField);
void avBitFieldDestroy(AvBitField* bitField);

bool8 avBitFieldTest(uint32 bit, AvBitField bitField);
void avBitFieldWrite(uint32 bit, bool8 state, AvBitField bitField);
void avBitFieldSet(uint32 bit, AvBitField bitField);
void avBitFieldReset(uint32 bit, AvBitField bitField);
void avBitFieldToggle(uint32 bit, AvBitField bitField);
uint32 avBitFieldCountOnes(AvBitField bitField);
uint32 avBitFieldCountZeros(AvBitField bitField);

void avBitFieldWriteAll(bool8 state, AvBitField bitField);


C_SYMBOLS_END
#endif//__AV_TIME__