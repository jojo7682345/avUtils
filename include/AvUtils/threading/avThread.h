#ifndef __AV_THREAD__
#define __AV_THREAD__
#include "../avDefinitions.h"
C_SYMBOLS_START

#include "../avTypes.h"

typedef struct AvThread_T* AvThread;

typedef int (*AvThreadEntry)(byte*, uint64);

typedef enum AvThreadState {
	AV_THREAD_STATE_UNINITALIZED = 0,
	AV_THREAD_STATE_STOPPED = 1,
	AV_THREAD_STATE_RUNNING = 2,
}AvThreadState;

typedef uint16 AvThreadID;
#define AV_MAX_THREADS (4096)
#define AV_MAIN_THREAD_ID (0)
#define AV_INVALID_THREAD_ID ((AvThreadID)-1)


void avThreadCreate(AvThreadEntry func, AvThread* thread);
bool8 avThreadStart(void* buffer, uint64 bufferSize, AvThread thread);
uint avThreadJoin(AvThread thread);
void avThreadDestroy(AvThread thread);

AvThreadID avThreadGetID();
void avThreadSleep(uint64 milis);
void avThreadSetName(AvThread thread, const char* name);
void avThreadYield();

C_SYMBOLS_END
#endif