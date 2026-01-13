#include <AvUtils/avEnvironment.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

bool32 avChangeDirectory(AvString path) {
    AvString str = AV_EMPTY;
    avStringClone(&str, path);

#ifndef _WIN32
    int ret = chdir(str.chrs);
#else
    int ret = SetCurrentDirectory(str.chrs) ? 0 : -1;
#endif
    avStringFree(&str);

    return ret == 0;

}

bool32 avGetEnvironmentVariable(AvString variable, AvStringRef value) {
    AvString str = AV_EMPTY;
    avStringClone(&str, variable);

#ifdef _WIN32
    char buff[4096] = { 0 };
    DWORD resultLengthInCharacters = GetEnvironmentVariable(str.chrs, buff, 4096);
    avStringFree(&str);
    if (resultLengthInCharacters == 0){
        return false;
    }
    avStringClone(value, AV_STR(buff, resultLengthInCharacters));
#else
    const char* env = getenv(str.chrs);
    avStringFree(&str);
    if (env == nullptr) {
        return false;
    }
    avStringClone(value, AV_CSTR(env));
#endif
    return true;
}