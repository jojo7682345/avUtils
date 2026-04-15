#include <AvUtils/threading/avMutex.h>
#include <AvUtils/threading/avConditionVariable.h>
#include <AvUtils/avMemory.h>

#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#endif


typedef struct AvMutex_T {
#ifdef _WIN32
    CRITICAL_SECTION mutexHandle;
#else
	pthread_mutex_t mutex;
#endif
} AvMutex_T;

static bool32 createMutex(AvMutex mutex);
static void destroyMutex(AvMutex mutex);
static void lockMutex(AvMutex mutex);
static void unlockMutex(AvMutex mutex);

bool32 avMutexCreate(AvMutex* mutex) {
	(*mutex) = avCallocate(1, sizeof(AvMutex_T), "allocating mutex");

	return createMutex(*mutex);
}

void avMutexDestroy(AvMutex mutex) {
	destroyMutex(mutex);

	avFree(mutex);
}

void avMutexLock(AvMutex mutex) {

	lockMutex(mutex);
}

void avMutexUnlock(AvMutex mutex) {
	unlockMutex(mutex);
}

#ifdef _WIN32
static bool32 createMutex(AvMutex mutex) {
    InitializeCriticalSection(&mutex->mutexHandle);
	// (mutex)->mutexHandle = CreateMutex(NULL, FALSE, NULL);
	// if ((mutex)->mutexHandle == NULL) {
	// 	printf("CreateMutex error: %ld\n", GetLastError());
	// 	return false;
	// }
	return true;
}

static void destroyMutex(AvMutex mutex) {
	//CloseHandle(mutex->mutexHandle);
    DeleteCriticalSection(&mutex->mutexHandle);
}

static void lockMutex(AvMutex mutex) {
	//WaitForSingleObject(mutex->mutexHandle, INFINITE);
    EnterCriticalSection(&mutex->mutexHandle);
}
static void unlockMutex(AvMutex mutex) {
	//ReleaseMutex(mutex->mutexHandle);
    LeaveCriticalSection(&mutex->mutexHandle);
}
#else
static bool32 createMutex(AvMutex mutex) {
    if(pthread_mutex_init(&mutex->mutex, NULL) != 0){
        return false;
    }
	return true;
}
static void destroyMutex(AvMutex mutex) {
    pthread_mutex_destroy(&mutex->mutex);
}

static void lockMutex(AvMutex mutex) {
	pthread_mutex_lock(&mutex->mutex);
}
static void unlockMutex(AvMutex mutex) {
	pthread_mutex_unlock(&mutex->mutex);
}
#endif

typedef struct AvConditionVariable_T {
#ifdef _WIN32
    CONDITION_VARIABLE condition;
#else
    pthread_cond_t condition;
#endif
} AvConditionVariable_T;

static bool32 createCondition(AvConditionVariable condition);
static void destroyCondition(AvConditionVariable condition);
static void waitCondition(AvConditionVariable condition, AvMutex mutex);
static bool32 timedWaitCondition(AvConditionVariable condition, AvMutex mutex, uint32 timeoutMs);
static void signalCondition(AvConditionVariable condition);
static void broadcastCondition(AvConditionVariable condition);

bool32 avConditionVariableCreate(AvConditionVariable* condition) {
    (*condition) = avCallocate(1, sizeof(AvConditionVariable_T), "allocating condition variable");
    return createCondition(*condition);
}

void avConditionVariableDestroy(AvConditionVariable condition) {
    destroyCondition(condition);
    avFree(condition);
}

void avConditionVariableWait(AvConditionVariable condition, AvMutex mutex) {
    waitCondition(condition, mutex);
}

bool32 avConditionVariableTimedWait(AvConditionVariable condition, AvMutex mutex, uint32 timeoutMs) {
    return timedWaitCondition(condition, mutex, timeoutMs);
}

void avConditionVariableSignal(AvConditionVariable condition) {
    signalCondition(condition);
}

void avConditionVariableBroadcast(AvConditionVariable condition) {
    broadcastCondition(condition);
}

#ifdef _WIN32

static bool32 createCondition(AvConditionVariable condition) {
    InitializeConditionVariable(&condition->condition);
    return true;
}

static void destroyCondition(AvConditionVariable condition) {
    // No-op on Windows
    (void)condition;
}

static void waitCondition(AvConditionVariable condition, AvMutex mutex) {
    SleepConditionVariableCS(
        &condition->condition,
        &mutex->mutexHandle,
        INFINITE
    );
}

static bool32 timedWaitCondition(AvConditionVariable condition, AvMutex mutex, uint32 timeoutMs) {
    BOOL result = SleepConditionVariableCS(
        &condition->condition,
        &mutex->mutexHandle,
        timeoutMs
    );
    return result != 0;
}

static void signalCondition(AvConditionVariable condition) {
    WakeConditionVariable(&condition->condition);
}

static void broadcastCondition(AvConditionVariable condition) {
    WakeAllConditionVariable(&condition->condition);
}

#else

static bool32 createCondition(AvConditionVariable condition) {
    if (pthread_cond_init(&condition->condition, NULL) != 0) {
        return false;
    }
    return true;
}

static void destroyCondition(AvConditionVariable condition) {
    pthread_cond_destroy(&condition->condition);
}

static void waitCondition(AvConditionVariable condition, AvMutex mutex) {
    pthread_cond_wait(&condition->condition, &mutex->mutex);
}

static bool32 timedWaitCondition(AvConditionVariable condition, AvMutex mutex, uint32 timeoutMs) {
    struct timespec ts;

    clock_gettime(CLOCK_REALTIME, &ts);

    ts.tv_sec += timeoutMs / 1000;
    ts.tv_nsec += (timeoutMs % 1000) * 1000000;

    if (ts.tv_nsec >= 1000000000) {
        ts.tv_sec += 1;
        ts.tv_nsec -= 1000000000;
    }

    int result = pthread_cond_timedwait(&condition->condition, &mutex->mutex, &ts);

    return result == 0;
}

static void signalCondition(AvConditionVariable condition) {
    pthread_cond_signal(&condition->condition);
}

static void broadcastCondition(AvConditionVariable condition) {
    pthread_cond_broadcast(&condition->condition);
}

#endif
