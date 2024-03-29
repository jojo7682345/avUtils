#include <AvUtils/avProcess.h>
#include <AvUtils/avMemory.h>
#include <AvUtils/avString.h>
#include <AvUtils/avLogging.h>
#include <AvUtils/dataStructures/avDynamicArray.h>
#include <AvUtils/dataStructures/avArray.h>
#include <AvUtils/avFileSystem.h>

#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#else
#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#endif

typedef uint64 PID;

typedef struct AvProcess_T {
    PID pid;
#ifdef _WIN32
    STARTUPINFO siStartInfo;
    PROCESS_INFORMATION piProcInfo;
#else

#endif
} AvProcess_T;

void avProcessStartInfoPopulate_(AvProcessStartInfo* info, AvString bin, AvString cwd, ...) {
    AvDynamicArray arr = AV_EMPTY;
    avDynamicArrayCreate(0, sizeof(AvString), &arr);
    va_list args;
    va_start(args, cwd);
    do {
        AvString arg = va_arg(args, AvString);
        if (arg.chrs == nullptr || arg.len == 0) {
            break;
        }
        avDynamicArrayAdd(&arg, arr);
    } while (1);
    va_end(args);

    uint32 argCount = avDynamicArrayGetSize(arr);
    AvString* argList = avCallocate(argCount, sizeof(AvString), "allocating args");
    avDynamicArrayForEachElement(AvString, arr, {
        memcpy(argList + index, &element, sizeof(AvString));
        });
    avDynamicArrayDestroy(arr);

    avProcessStartInfoCreate(info, bin, cwd, argCount, argList);
    avFree(argList);
}

void avProcessStartInfoCreate(AvProcessStartInfo* info, AvString bin, AvString cwd, uint32 argCount, AvString* argValues) {
    avAssert(info != nullptr, "info must be a valid pointer");
    avAssert(bin.len != 0, "executable cannot be empty");

    avStringClone(&info->executable, bin);
    if (cwd.len != 0) {
        avStringClone(&info->workingDirectory, cwd);
    }

    avArrayAllocate(argCount, sizeof(AvString), &info->args);
    for (uint32 i = 0; i < argCount; i++) {
        avStringClone(((AvString*)info->args.data) + i, argValues[i]);
    }
}

void avProcessStartInfoDestroy(AvProcessStartInfo* info) {
    avAssert(info != nullptr, "info must be a valid pointer");
    avStringFree(&info->executable);
    avStringFree(&info->workingDirectory);
    avArrayFreeAndDestroy(&info->args, (AvDestroyElementCallback)avStringFree);
}


static void configureProcess(AvProcessStartInfo info, AvProcess process);
static bool32 executeProcess(AvProcessStartInfo info, AvProcess process);
bool32 waitForProcess(AvProcess process, int32* exitStatus);
static void killProcess(AvProcess process);

bool32 avProcessStart(AvProcessStartInfo info, AvProcess* process) {
    (*process) = avCallocate(1, sizeof(AvProcess_T), "allocating process");
    configureProcess(info, *process);
    return executeProcess(info, *process);
}

int32 avProcessRun(AvProcessStartInfo info) {
    AvProcess process = AV_EMPTY;
    if (!avProcessStart(info, &process)) {
        return -1;
    }
    int32 ret = avProcessWaitExit(process);
    avProcessDiscard(process);
    return ret;
}

int32 avProcessWaitExit(AvProcess process) {
    int32 exitStatus = -1;
    if (!waitForProcess(process, &exitStatus)) {
        avAssert(false, "failed to get process status, pobably an invalid process handle");
        return exitStatus;
    }
    return exitStatus;
}
void avProcessKill(AvProcess process) {
    killProcess(process);
    avProcessDiscard(process);
}
void avProcessDiscard(AvProcess process) {
    avFree(process);
}

#ifdef _WIN32
static LPSTR GetLastErrorAsString(void) {

    DWORD errorMessageId = GetLastError();
    avAssert(errorMessageId != 0, "");

    LPSTR messageBuffer = NULL;

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, // DWORD   dwFlags,
        NULL, // LPCVOID lpSource,
        errorMessageId, // DWORD   dwMessageId,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // DWORD   dwLanguageId,
        (LPSTR)&messageBuffer, // LPTSTR  lpBuffer,
        0, // DWORD   nSize,
        NULL // va_list *Arguments
    );

    return messageBuffer;
}



static void configureProcess(AvProcessStartInfo startInfo, AvProcess process) {
    ZeroMemory(&process->siStartInfo, sizeof(process->siStartInfo));
    process->siStartInfo.cb = sizeof(STARTUPINFO);
    process->siStartInfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    process->siStartInfo.hStdOutput = (startInfo.output != nullptr) ? (HANDLE)_get_osfhandle(*startInfo.output) : GetStdHandle(STD_OUTPUT_HANDLE);
    process->siStartInfo.hStdInput = (startInfo.input != nullptr) ? (HANDLE)_get_osfhandle(*startInfo.input) : GetStdHandle(STD_INPUT_HANDLE);
    process->siStartInfo.dwFlags |= STARTF_USESTDHANDLES;
    ZeroMemory(&process->piProcInfo, sizeof(PROCESS_INFORMATION));
}

static bool32 executeProcess(AvProcessStartInfo info, AvProcess process) {
    avStringDebugContextStart;
    AvStringMemory memory = AV_EMPTY;
    uint32 commandLength = 0;
    commandLength += info.executable.len;
    avArrayForEachElement(AvString, arg, i, &info.args, {
        commandLength += 1 + arg.len;
        });
    avStringMemoryAllocate(commandLength, &memory);
    avStringMemoryStore(info.executable, 0, AV_STRING_FULL_LENGTH, &memory);
    uint32 index = info.executable.len;
    avArrayForEachElement(AvString, arg, i, &info.args, {
        avStringMemoryStore(AV_STR(" ", 1), index++, 1, &memory);
        avStringMemoryStore(arg, index, AV_STRING_FULL_LENGTH, &memory);
        index += arg.len;
        });
    AvString cmd = AV_EMPTY;
    avStringFromMemory(&cmd, AV_STRING_WHOLE_MEMORY, &memory);

    int32 bSuccess = CreateProcessA(
        NULL,
        (char*)cmd.chrs,
        NULL,
        NULL, 
        TRUE,
        0,
        NULL,
        info.workingDirectory.chrs,
        (LPSTARTUPINFOA)&process->siStartInfo,
        &process->piProcInfo
    );

    avStringFree(&cmd);
    if (!bSuccess) {
        printf("Could not create child process %s: %s\n",
            cmd.chrs, GetLastErrorAsString());
        avStringDebugContextEnd;
        return false;
    }
    CloseHandle(process->piProcInfo.hThread);
    process->pid = (PID)(process->piProcInfo.hProcess);

    avStringDebugContextEnd;
    return true;
}

static void killProcess(AvProcess process) {
    TerminateProcess((HANDLE)(process->pid), 0);
}


bool32 waitForProcess(AvProcess process, int32* exitStatus) {
    DWORD result = WaitForSingleObject(
        (HANDLE)process->pid,     // HANDLE hHandle,
        INFINITE // DWORD  dwMilliseconds
    );

    if (result == WAIT_FAILED) {
        //TODO: add logging
        printf("could not wait on child process: %s", GetLastErrorAsString());
        return false;
    }

    if (GetExitCodeProcess((HANDLE)process->pid, (LPDWORD)exitStatus) == 0) {
        printf("could not get process exit code: %lu", GetLastError());
        return false;
    }

    CloseHandle((HANDLE)process->pid);
    return true;
}

#else

static void configureProcess(AvProcess process) {
    printf("still has to be implemented\n");
    //TODO: implement
    return;
}

static bool32 waitForProcess(AvProcess process) {
    printf("still has to be implemented\n");
    //TODO: implement
    return false;
}

static bool32 executeProcess(AvString cmd, AvProcess process) {
    printf("still has to be implemented\n");
    // TODO: implement
    //  PID cpid = fork();
    //  if (cpid < 0) {
    //      printf("Could not fork child process: %s: %s", process->program, strerror(errno));
    //      return false;
    //  }

    // if (cpid == 0) {


    //     // if (fdin) {
    //     //     if (dup2(*fdin, STDIN_FILENO) < 0) {
    //     //         printf("Could not setup stdin for child process: %s", strerror(errno));
    //     //     }
    //     // }

    //     // if (fdout) {
    //     //     if (dup2(*fdout, STDOUT_FILENO) < 0) {
    //     //         printf("Could not setup stdout for child process: %s", strerror(errno));
    //     //     }
    //     // }
    //     // if (execvp(args.elems[0], (char* const*)args.elems) < 0) {
    //     //     printf("Could not exec child process: %s: %s",
    //     //           cmd, strerror(errno));
    //     // }
    // }

    return true;//cpid;
}

#endif