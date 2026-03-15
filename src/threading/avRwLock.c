#include <AvUtils/threading/avRwLock.h>
#include <AvUtils/avMemory.h>

#ifdef _WIN32
#include <windows.h>
#else
#define _POSIX_C_SOURCE 200809L
#include <pthread.h>
#endif

typedef struct AvRwLock_T {
#ifdef _WIN32
    SRWLOCK lock;
#else
    pthread_rwlock_t lock;
#endif
} AvRwLock_T;

bool32 avRWLockCreate(AvRwLock* lock) {
    *lock = avCallocate(1, sizeof(AvRwLock_T), "allocating rwlock");

#ifdef _WIN32
    InitializeSRWLock(&(*lock)->lock);
    return true;
#else
    return pthread_rwlock_init(&(*lock)->lock, NULL) == 0;
#endif
}

void avRWLockDestroy(AvRwLock lock) {
#ifdef _WIN32
    // SRWLOCK does not require destruction
#else
    pthread_rwlock_destroy(&lock->lock);
#endif
    avFree(lock);
}

void avRWLockReadLock(AvRwLock lock) {
#ifdef _WIN32
    AcquireSRWLockShared(&lock->lock);
#else
    pthread_rwlock_rdlock(&lock->lock);
#endif
}

void avRWLockReadUnlock(AvRwLock lock) {
#ifdef _WIN32
    ReleaseSRWLockShared(&lock->lock);
#else
    pthread_rwlock_unlock(&lock->lock);
#endif
}

void avRWLockWriteLock(AvRwLock lock) {
#ifdef _WIN32
    AcquireSRWLockExclusive(&lock->lock);
#else
    pthread_rwlock_wrlock(&lock->lock);
#endif
}

void avRWLockWriteUnlock(AvRwLock lock) {
#ifdef _WIN32
    ReleaseSRWLockExclusive(&lock->lock);
#else
    pthread_rwlock_unlock(&lock->lock);
#endif
}