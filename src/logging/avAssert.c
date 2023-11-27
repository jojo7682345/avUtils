#include <AvUtils/logging/avAssert.h>
#include <stdio.h>
#include <stdlib.h>

//TODO implement propper logging

void avAssertFailed(bool32 condition, const char* expression, const char* message, uint64 line, const char* function, const char* file) {
	printf("assert failed:\n\t%s\n\tfunc:\t%s\n\tline:\t%lu\n\tfile:\t%s\n\t%s\n", expression, function, line, file, message);
	exit(1);
}
