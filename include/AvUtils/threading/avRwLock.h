#ifndef __AV_RW_LOCK__
#define __AV_RW_LOCK__
#include "../avDefinitions.h"
C_SYMBOLS_START

#include "../avTypes.h"

typedef struct AvRwLock_T* AvRwLock;

bool32 avRWLockCreate(AvRwLock* lock);
void avRWLockDestroy(AvRwLock lock);

void avRWLockReadLock(AvRwLock lock);
void avRWLockReadUnlock(AvRwLock lock);

void avRWLockWriteLock(AvRwLock lock);
void avRWLockWriteUnlock(AvRwLock lock);

C_SYMBOLS_END
#endif//__AV_MUTEX__