#ifndef __AV_PROCESS__
#define __AV_PROCESS__

#include "avTypes.h"

typedef struct AvProcess_T* AvProcess;
typedef void* PID;

typedef void* FileDescriptor; //TODO: change to custom implementation

void avProcessCreate(const char* exec, FileDescriptor* input, FileDescriptor* output, AvProcess* process);

bool32 avProcessStart(uint32 argC, const char* argV[], AvProcess process);

bool32 avProcessWaitForTermination(uint32* retCode, AvProcess process);

void avProcessDestroy(AvProcess process);

#endif//__AV_PROCESS__