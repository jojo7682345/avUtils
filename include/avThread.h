#ifndef __AV_THREAD__
#define __AV_THREAD__

#include "avTypes.h"

typedef struct AvThread_T* AvThread;

typedef int (*AvThreadEntry)(byte*, uint64);

typedef enum AvThreadState {
	AV_THREAD_STATE_UNINITALIZED = 0,
	AV_THREAD_STATE_STOPPED = 1,
	AV_THREAD_STATE_RUNNING = 2,
}AvThreadState;

void avCreateThread(AvThreadEntry func, AvThread* thread);
bool8 avStartThread(byte* buffer, uint64 bufferSize, AvThread thread);
uint avJoinThread(AvThread thread);
void avDestroyThread(AvThread thread);

#endif