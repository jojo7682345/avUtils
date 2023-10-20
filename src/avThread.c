#include <avThread.h>
#define AV_ALLOCATE_ZERO_OUT
#include <avMemory.h>
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#include <strsafe.h>
#else
#include <pthread.h>

#endif

typedef struct AvThread_T {
	AvThreadEntry entry;
	AvThreadState state;
	byte* buffer;
	uint64 bufferSize;

#ifdef _WIN32
	DWORD threadID;
	HANDLE threadHandle;
#else
	pthread_t threadHandle;
#endif
} AvThread_T;


bool8 startThread(AvThread thread);
uint joinThread(AvThread thread);

void avCreateThread(AvThreadEntry func, AvThread* thread) {

	(*thread) = avAllocate(sizeof(AvThread_T), "allocating handle for thread");
	(*thread)->entry = func;
	(*thread)->state = AV_THREAD_STATE_STOPPED;

}

bool8 avStartThread(byte* buffer, uint64 bufferSize, AvThread thread) {

	if (thread->state != AV_THREAD_STATE_STOPPED) {
		printf("thread must not be in the running state\n");
		return false;
	}

	thread->buffer = buffer;
	thread->bufferSize = bufferSize;

	if (!startThread(thread)) {
		return false;
	}

	thread->state = AV_THREAD_STATE_RUNNING;
	return true;

}

uint avJoinThread(AvThread thread) {
	if (thread->state != AV_THREAD_STATE_RUNNING) {
		printf("thread must be running in order to be joined\n");
		return (uint)-1;
	}
	uint ret = joinThread(thread);

	thread->state = AV_THREAD_STATE_STOPPED;

	return ret;
}

void avDestroyThread(AvThread thread) {
	thread->state = AV_THREAD_STATE_UNINITALIZED;
	avFree(thread);
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
        (LPTSTR) &lpMsgBuf,
        0, NULL );

    // Display the error message and exit the process

    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, 
        (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR)); 
    StringCchPrintf((LPTSTR)lpDisplayBuf, 
        LocalSize(lpDisplayBuf) / sizeof(TCHAR),
        TEXT("%s failed with error %d: %s"), 
        lpszFunction, dw, lpMsgBuf); 
    MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK); 

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
    ExitProcess(dw); 
}


DWORD WINAPI run(LPVOID lpParam) {
	AvThread thread = (AvThread)lpParam;
	uint exitCode = thread->entry(thread->buffer, thread->bufferSize);
	return (DWORD) exitCode;
}

bool8 startThread(AvThread thread) {
	HANDLE threadHandle = CreateThread(NULL, 0, &run, thread, 0, &(thread->threadID));
	if (threadHandle == NULL) {
		printf("failed to start thread\n");
		return false;
	}
	thread->threadHandle = threadHandle;	
	return true;
}

uint joinThread(AvThread thread) {
	if (CloseHandle(thread->threadHandle)) {
		handleWinError(TEXT("closing thread"));
	}
	DWORD exitCode = 0;
	if (GetExitCodeThread(thread->threadHandle, &exitCode)) {
		handleWinError(TEXT("getting thread exit code"));
	}
	return (uint)exitCode;
}


#else
#include <unistd.h>

void* run(void* arg){
	AvThread thread = (AvThread) arg;
	uint ret = thread->entry(thread->buffer, thread->bufferSize);
	pthread_exit((void*)(unsigned long)ret);
}

bool8 startThread(AvThread thread) {
	int res = pthread_create(&thread->threadHandle, NULL, &run, thread);
	if(res){
		return false;
	}
	return true;
}

uint joinThread(AvThread thread) {
	void* retCode = 0;
	if(pthread_join(thread->threadHandle, &retCode)){
		return (uint)-1;
	}

	return (uint)(unsigned long)retCode;
}

#endif


