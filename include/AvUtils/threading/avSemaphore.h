#ifndef __AV_SEMAPHORE__
#define __AV_SEMAPHORE__

#include "../avDefinitions.h"
C_SYMBOLS_START

#include "../avTypes.h"

typedef struct AvSemaphore_T* AvSemaphore;

// initialCount = starting value of the semaphore
bool32 avSemaphoreCreate(AvSemaphore* semaphore, uint32 initialCount);

void avSemaphoreDestroy(AvSemaphore semaphore);

// decrement (wait)
void avSemaphoreWait(AvSemaphore semaphore);

// increment (signal)
void avSemaphorePost(AvSemaphore semaphore);

void avSemaphorePostCount(AvSemaphore semaphore, uint32 count);

// non-blocking (returns false if it would block)
bool32 avSemaphoreTryWait(AvSemaphore semaphore);

// timed wait (timeout in milliseconds)
// returns false if timeout occurs
bool32 avSemaphoreTimedWait(AvSemaphore semaphore, uint32 timeoutMs);


C_SYMBOLS_END
#endif // __AV_SEMAPHORE__