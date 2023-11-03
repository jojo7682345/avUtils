#ifndef __AV_FIXED_MAP__
#define __AV_FIXED_MAP__

#include "../avTypes.h"

typedef struct AvFMap_T* AvFMap;

#define AV_MAP_STORED_KEY_SIZE 0

typedef uint32(*HashFunction)(void* data, uint64 dataSize, uint32 mapSize);

void avFMapCreate(uint32 size, uint64 dataSize, uint64 keySize, AvFMap* map, HashFunction hashFunction = NULL);

void avFMapWrite(void* data, void* key, AvFMap map, uint64 keySize = AV_MAP_STORED_KEY_SIZE);
void avFMapRead(void* data, void* key, AvFMap map, uint64 keySize = AV_MAP_STORED_KEY_SIZE);

void* avFMapGetPtr(void* key, AvFMap map, uint64 keySize = AV_MAP_STORED_KEY_SIZE);

uint32 avFMapGetSize(AvFMap map);
uint64 avFMapGetDataSize(AvFMap map);
uint64 avFMapGetKeySize(AvFMap map);

void avFMapClear(void* data, AvFMap map);

void avFMapDestroy(AvFMap map);

#endif//__AV_MAP__