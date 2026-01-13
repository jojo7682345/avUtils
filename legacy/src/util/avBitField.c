#include <AvUtils/util/avBitfield.h>
#include <AvUtils/logging/avAssert.h>
#include <AvUtils/avMemory.h>

#include <string.h>

#define MASK_ONES(remainder) (((byte)0xff)>>(8-(remainder)))

static inline uint32 getByteCount(uint32 bitCount){
    return bitCount/8+(bitCount%8!=0);
}

void avBitFieldCreate(uint32 bitCount, AvBitField* bitField){
    avAssert(bitField!=nullptr, "bitField must be a valid pointer");
    avAssert(bitCount>0, "bitfield cannot have a size of 0");

    AvBitField field = {
        .bitCount = bitCount,
        .bits = avCallocate(getByteCount(bitCount), 1, "allocating bitfield")
    };
    memcpy(bitField, &field, sizeof(AvBitField));
}

void avBitFieldDestroy(AvBitField* bitField){
    avAssert(bitField!=nullptr, "bitField must be a valid pointer");
    avAssert(bitField->bitCount>0, "bitfield cannot have a size of 0");
    avFree(bitField->bits);
    memset(bitField, 0, sizeof(AvBitField));
}

bool8 avBitFieldTest(uint32 bit, AvBitField bitField){
    if(bit >= bitField.bitCount){
        return false;
    }
    return bitField.bits[bit/8] & (1<<(bit%8));
}

void avBitFieldWrite(uint32 bit, bool8 state, AvBitField bitField){
    if(bit >= bitField.bitCount){
        return;
    }
    bitField.bits[bit/8] |= (state?1:0)<<(bit%8);
}

void avBitFieldSet(uint32 bit, AvBitField bitField){
    avBitFieldWrite(bit, 1, bitField);
}

void avBitFieldReset(uint32 bit, AvBitField bitField){
    avBitFieldWrite(bit, 0, bitField);
}

void avBitFieldToggle(uint32 bit, AvBitField bitField){
    avBitFieldWrite(bit, !avBitFieldTest(bit, bitField), bitField);
}

static uint8 count_ones (uint8 byte)
{
  static const uint8 NIBBLE_LOOKUP [16] =
  {
    0, 1, 1, 2, 1, 2, 2, 3, 
    1, 2, 2, 3, 2, 3, 3, 4
  };


  return NIBBLE_LOOKUP[byte & 0x0F] + NIBBLE_LOOKUP[byte >> 4];
}

uint32 avBitFieldCountOnes(AvBitField bitField){
    uint32 count = 0;
    byte remainder = bitField.bitCount % 8;
    for(uint32 i = 0; i < getByteCount(bitField.bitCount); i++){
        byte data = bitField.bits[i];
        if(remainder && i == bitField.bitCount/8){
            data &= MASK_ONES(remainder);
        }
        count+= count_ones(data);
    }
    return count;
}

uint32 avBitFieldCountZeros(AvBitField bitField){
    uint32 count = bitField.bitCount;
    return count - avBitFieldCountOnes(bitField);
}



void avBitFieldWriteAll(bool8 state, AvBitField bitField){
    memset(bitField.bits, state?(0xff):(0x00), getByteCount(bitField.bitCount));
    if(state){
        bitField.bits[getByteCount(bitField.bitCount)] &= MASK_ONES(bitField.bitCount % 8);
    }
}
