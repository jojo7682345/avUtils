#include <AvUtils/threading/avSemaphore.h>
#include <AvUtils/avMemory.h>

#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <semaphore.h>
#endif

typedef struct AvSemaphore_T {
#ifdef _WIN32
    HANDLE semaphoreHandle;
#else
    sem_t semaphore;
#endif
} AvSemaphore_T;

static bool32 createSemaphore(AvSemaphore semaphore, uint32 initialCount);
static void destroySemaphore(AvSemaphore semaphore);
static void waitSemaphore(AvSemaphore semaphore);
static void postSemaphore(AvSemaphore semaphore);
static bool32 tryWaitSemaphore(AvSemaphore semaphore);
static bool32 timedWaitSemaphore(AvSemaphore semaphore, uint32 timeoutMs);

bool32 avSemaphoreCreate(AvSemaphore* semaphore, uint32 initialCount) {
    (*semaphore) = avCallocate(1, sizeof(AvSemaphore_T), "allocating semaphore");
    return createSemaphore(*semaphore, initialCount);
}

void avSemaphoreDestroy(AvSemaphore semaphore) {
    destroySemaphore(semaphore);
    avFree(semaphore);
}

void avSemaphoreWait(AvSemaphore semaphore) {
    waitSemaphore(semaphore);
}

bool32 avSemaphoreTryWait(AvSemaphore semaphore) {
    return tryWaitSemaphore(semaphore);
}

bool32 avSemaphoreTimedWait(AvSemaphore semaphore, uint32 timeoutMs) {
    return timedWaitSemaphore(semaphore, timeoutMs);
}

void avSemaphorePost(AvSemaphore semaphore) {
    postSemaphore(semaphore);
}

#ifdef _WIN32

static bool32 createSemaphore(AvSemaphore semaphore, uint32 initialCount) {
    semaphore->semaphoreHandle = CreateSemaphore(
        NULL,
        initialCount,
        LONG_MAX,
        NULL
    );

    if (semaphore->semaphoreHandle == NULL) {
        printf("CreateSemaphore error: %ld\n", GetLastError());
        return false;
    }
    return true;
}

static void destroySemaphore(AvSemaphore semaphore) {
    CloseHandle(semaphore->semaphoreHandle);
}

static void waitSemaphore(AvSemaphore semaphore) {
    WaitForSingleObject(semaphore->semaphoreHandle, INFINITE);
}

static bool32 tryWaitSemaphore(AvSemaphore semaphore) {
    DWORD result = WaitForSingleObject(semaphore->semaphoreHandle, 0);
    return result == WAIT_OBJECT_0;
}

static bool32 timedWaitSemaphore(AvSemaphore semaphore, uint32 timeoutMs) {
    DWORD result = WaitForSingleObject(semaphore->semaphoreHandle, timeoutMs);
    return result == WAIT_OBJECT_0;
}

static void postSemaphore(AvSemaphore semaphore) {
    ReleaseSemaphore(semaphore->semaphoreHandle, 1, NULL);
}

#else

static bool32 createSemaphore(AvSemaphore semaphore, uint32 initialCount) {
    if (sem_init(&semaphore->semaphore, 0, initialCount) != 0) {
        perror("sem_init failed");
        return false;
    }
    return true;
}

static void destroySemaphore(AvSemaphore semaphore) {
    sem_destroy(&semaphore->semaphore);
}

static void waitSemaphore(AvSemaphore semaphore) {
    sem_wait(&semaphore->semaphore);
}

static bool32 tryWaitSemaphore(AvSemaphore semaphore) {
    int result = sem_trywait(&semaphore->semaphore);
    return result == 0;
}

static bool32 timedWaitSemaphore(AvSemaphore semaphore, uint32 timeoutMs) {
    struct timespec ts;

    // get current time
    clock_gettime(CLOCK_REALTIME, &ts);

    // add timeout
    ts.tv_sec += timeoutMs / 1000;
    ts.tv_nsec += (timeoutMs % 1000) * 1000000;

    // normalize
    if (ts.tv_nsec >= 1000000000) {
        ts.tv_sec += 1;
        ts.tv_nsec -= 1000000000;
    }

    while (1) {
        int result = sem_timedwait(&semaphore->semaphore, &ts);
        if (result == 0) {
            return true;
        }

        if (errno == ETIMEDOUT) {
            return false;
        }

        if (errno != EINTR) {
            return false;
        }
        // else retry if interrupted
    }
}

static void postSemaphore(AvSemaphore semaphore) {
    sem_post(&semaphore->semaphore);
}

#endif