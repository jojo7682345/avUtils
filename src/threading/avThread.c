#include <AvUtils/threading/avThread.h>
#define AV_ALLOCATE_ZERO_OUT
#include <AvUtils/avMemory.h>
#include <stdio.h>
#include <stdatomic.h>

#ifdef _WIN32
#include <Windows.h>
#include <strsafe.h>
#include <process.h>
#include <processthreadsapi.h>
#else
#ifndef __USE_MISC
#define __USE_MISC
#endif
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#endif
#include <AvUtils/avString.h>

typedef struct AvThread_T {
	AvThreadEntry entry;
	AvThreadState state;
	byte* buffer;
	uint64 bufferSize;
    AvString name;

#ifdef _WIN32
	DWORD threadID;
	HANDLE threadHandle;
#else
	pthread_t threadHandle;
#endif
} AvThread_T;


static uint16 freeThreadIds[AV_MAX_THREADS-1];
static _Atomic uint16 freeThreadTop = AV_MAX_THREADS-1; //MAIN_THREAD WILL always be 0
static bool8 threadIDsInitialized = false;

#ifdef _MSC_VER
    __declspec(thread) static AvThreadID currentThreadId;
#elif defined(__GNUC__)
    static __thread AvThreadID currentThreadId;
#else
    static _Thread_local AvThreadID currentThreadId;
#endif

static void initThreadIDs(){
    for(uint32 i = 0; i < AV_MAX_THREADS-1; i++){
        freeThreadIds[i] = AV_MAX_THREADS - 1 - i; // no -2 as the main thread is already allocated
    }
    threadIDsInitialized = true;
}

static AvThreadID allocateThreadId(){
    uint16 top = 0;
    while(1){
        top = atomic_load(&freeThreadTop);
        if(top==0){
            return AV_INVALID_THREAD_ID;
        }
        uint16 newTop = top - 1;

        if(atomic_compare_exchange_weak(
            &freeThreadTop,
            &top,
            newTop)){
            
                return freeThreadIds[newTop];
        }
    }
}

static void freeThreadId(AvThreadID id){
    uint16 top = 0;
    while(1){
        top = atomic_load(&freeThreadTop);
        if(top >= AV_MAX_THREADS){
            return;
        }

        freeThreadIds[top] = id;

        uint16 newTop = top + 1;
        if(atomic_compare_exchange_weak(
            &freeThreadTop,
            &top,
            newTop)){
                
            return;
        }
    }
}


bool8 startThread(AvThread thread);
uint joinThread(AvThread thread);

void sleepThread(uint64 milis);
void renameThread(AvThread thread, const char* name);

void avThreadCreate(AvThreadEntry func, AvThread* thread) {

    if(threadIDsInitialized) initThreadIDs();

	(*thread) = avAllocate(sizeof(AvThread_T), "allocating handle for thread");
	(*thread)->entry = func;
    avStringInitialize(&(*thread)->name);
	(*thread)->state = AV_THREAD_STATE_STOPPED;

}

bool8 avThreadStart(void* buffer, uint64 bufferSize, AvThread thread) {

	if (thread->state != AV_THREAD_STATE_STOPPED) {
		printf("thread must not be in the running state\n");
		return false;
	}

	thread->buffer = buffer;
	thread->bufferSize = bufferSize;

	if (!startThread(thread)) {
		return false;
	}
    if(!avStringIsEmpty(thread->name)){
        renameThread(thread, thread->name.chrs);
    }
	thread->state = AV_THREAD_STATE_RUNNING;
	return true;

}

uint avThreadJoin(AvThread thread) {
	if (thread->state != AV_THREAD_STATE_RUNNING) {
		printf("thread must be running in order to be joined\n");
		return (uint)-1;
	}
	uint ret = joinThread(thread);

	thread->state = AV_THREAD_STATE_STOPPED;

	return ret;
}

void avThreadDestroy(AvThread thread) {
	if (thread->state == AV_THREAD_STATE_RUNNING) {
		avThreadJoin(thread);
	}
    avStringFree(&thread->name);
	thread->state = AV_THREAD_STATE_UNINITALIZED;
	avFree(thread);
}

void avThreadSleep(uint64 milis) {
	sleepThread(milis);
}

AvThreadID avThreadGetID(){
    return currentThreadId;
}

void avThreadSetName(AvThread thread, const char* name){
    AvString tmp;
    avStringCopySection(&tmp, 0, 15, AV_CSTR(name));
    avStringClone(&thread->name, tmp);
    avStringFree(&tmp);
    if(thread->state!=AV_THREAD_STATE_RUNNING){
        renameThread(thread, thread->name.chrs);
    }
}

#ifdef _WIN32

void handleWinError(LPTSTR lpszFunction) {
	// Retrieve the system error message for the last-error code

	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;
	DWORD dw = GetLastError();
	if (dw == 0) {
		return;
	}
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	// Display the error message and exit the process

	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
		(lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
	sprintf_s((LPTSTR)lpDisplayBuf,
		LocalSize(lpDisplayBuf) / sizeof(TCHAR),
		TEXT("%s failed with error %d: %s"),
		lpszFunction, dw, lpMsgBuf);
	MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);
	ExitProcess(dw);
}


uint32 WINAPI run(void* lpParam) {
	AvThread thread = (AvThread)lpParam;
    currentThreadId = allocateThreadId();
    if(currentThreadId==AV_INVALID_THREAD_ID) return -1;
	uint exitCode = thread->entry(thread->buffer, thread->bufferSize);
	_endthreadex((DWORD)exitCode);
    freeThreadId(currentThreadId);
    currentThreadId = AV_INVALID_THREAD_ID;
	return (DWORD)exitCode;
}

bool8 startThread(AvThread thread) {
	HANDLE threadHandle = (HANDLE)_beginthreadex(NULL, 0, &run, thread, 0, 0);
	if (threadHandle == NULL) {
		printf("failed to start thread\n");
		return false;
	}
	if (threadHandle == INVALID_HANDLE_VALUE) {
		printf("invalid handle value");
		return false;
	}
	thread->threadHandle = threadHandle;
	return true;
}

uint joinThread(AvThread thread) {
	
	switch (WaitForSingleObject(thread->threadHandle, INFINITE)) {
	case WAIT_OBJECT_0:
		
		break;
	case WAIT_ABANDONED:
	case WAIT_TIMEOUT:
	case WAIT_FAILED:
		handleWinError(TEXT("waiting"));
		break;
	default:
		break;
	}
	DWORD exitCode = 0;
	if (!GetExitCodeThread(thread->threadHandle, &exitCode)) {
		handleWinError(TEXT("getting thread exit code"));
	}
	if (!CloseHandle(thread->threadHandle)) {
		handleWinError(TEXT("closing thread"));
	}

	return (uint)exitCode;
}

void sleepThread(uint64 milis) {
	Sleep((DWORD)milis);
}

void renameThread(AvThread thread, const char* name){
    if (!thread || !thread->threadHandle) return;

    wchar_t wname[64];
    MultiByteToWideChar(CP_UTF8, 0, name, -1, wname, 64);

    SetThreadDescription(thread->threadHandle, wname);
}


#else

void* run(void* arg) {
	AvThread thread = (AvThread)arg;
    currentThreadId = allocateThreadId();
    if(currentThreadId==AV_INVALID_THREAD_ID) return -1;
	uint ret = thread->entry(thread->buffer, thread->bufferSize);
    freeThreadId(currentThreadId);
    currentThreadId = AV_INVALID_THREAD_ID;
	pthread_exit((void*)(unsigned long)ret);
}

bool8 startThread(AvThread thread) {
	int res = pthread_create(&thread->threadHandle, NULL, &run, thread);
	if (res) {
		return false;
	}
	return true;
}

uint joinThread(AvThread thread) {
	void* retCode = 0;
	if (pthread_join(thread->threadHandle, &retCode)) {
		return (uint)-1;
	}

	return (uint)(unsigned long)retCode;
}

void sleepThread(uint64 milis) {
	usleep(milis * 1000U);
}

void renameThread(AvThread thread, const char* name){
    if (!thread || thread.) return;

    pthread_setname_np(thread->threadHandle, name);
}

#endif


