#include <AvUtils/avProcess.h>
#include <AvUtils/avMemory.h>
#include <AvUtils/avString.h>
#include <AvUtils/avLogging.h>

#include <stdio.h>
#include <unistd.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#endif

typedef struct AvProcess_T {
    FileDescriptor* input;
    FileDescriptor* output;
    PID pid;
    uint32 retCode;
    const char* program;
#ifdef _WIN32
    STARTUPINFO siStartInfo;
    PROCESS_INFORMATION piProcInfo;
#else

#endif
} AvProcess_T;

static void configureProcess(AvProcess process);
static bool32 executeProcess(AvString cmd, AvProcess process);
static bool32 waitForProcess(AvProcess process);

void avProcessCreate(const char* executable, FileDescriptor* input, FileDescriptor* output, AvProcess* process) {
    (*process) = avCallocate(1, sizeof(AvProcess_T), "allocating process handle");
    (*process)->input = input;
    (*process)->output = output;
    (*process)->program = executable;
    configureProcess(*process);
}


bool32 avProcessStart(uint32 argC, const char* argV[], AvProcess process) {
    avStringDebugContextStart;

    AvStringMemory memory;
    avStringMemoryStoreCharArrays(&memory, argC, argV);
    AvString arguments;
    avStringFromMemory(&arguments, 0, AV_STRING_FULL_LENGTH, &memory);
    AvString cmd;
    avStringJoin(&cmd, AV_CSTR(process->program), arguments);
    avStringFree(&arguments);
    bool32 success = executeProcess(cmd, process);
    avStringFree(&cmd);

    avStringDebugContextEnd;
    return success;
}

bool32 avProcessWaitForTermination(uint32* retCode, AvProcess process){
    if(waitForProcess(process)){
        if(retCode){
            *retCode = process->retCode;
        }
        return true;
    }
    return false;
}

void avProcessDestroy(AvProcess process){
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



static void configureProcess(AvProcess process) {
    ZeroMemory(&process->siStartInfo, sizeof(process->siStartInfo));
    process->siStartInfo.cb = sizeof(STARTUPINFO);
    process->siStartInfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    process->siStartInfo.hStdOutput = process->output ? *process->output : GetStdHandle(STD_OUTPUT_HANDLE);
    process->siStartInfo.hStdInput = process->input ? *process->input : GetStdHandle(STD_INPUT_HANDLE);
    process->siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

    ZeroMemory(&process->piProcInfo, sizeof(PROCESS_INFORMATION));
}

static bool32 executeProcess(AvString cmd, AvProcess process) {

    int32 bSuccess = CreateProcessA(
        NULL,
        (char*)cmd.chrs,
        NULL,
        NULL,
        TRUE,
        0,
        NULL,
        NULL,
        (LPSTARTUPINFOA)&process->siStartInfo,
        &process->piProcInfo
    );
    if (!bSuccess) {
        printf("Could not create child process %s: %s\n",
              cmd.chrs, GetLastErrorAsString());
        return false;
    }
    CloseHandle(process->piProcInfo.hThread);
    process->pid = process->piProcInfo.hProcess;

    return true;
}



bool32 waitForProcess(AvProcess process){
     DWORD result = WaitForSingleObject(
        process->pid,     // HANDLE hHandle,
        INFINITE // DWORD  dwMilliseconds
    );

    if (result == WAIT_FAILED) {
        //TODO: add logging
        printf("could not wait on child process: %s", GetLastErrorAsString());
        return false;
    }

    DWORD exit_status;
    if (GetExitCodeProcess(process->pid, &exit_status) == 0) {
        printf("could not get process exit code: %lu", GetLastError());
        return false;
    }

    process->retCode = exit_status;

    CloseHandle(process->pid);
    return true;
}

#else

static void configureProcess(AvProcess process){
    printf("still has to be implemented\n");
    //TODO: implement
    return;
}

static bool32 waitForProcess(AvProcess process){
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