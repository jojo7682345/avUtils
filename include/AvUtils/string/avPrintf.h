#ifndef __AV_PRINTF__
#define __AV_PRINTF__
#include "../avDefinitions.h"
C_SYMBOLS_START
#include "../avTypes.h"
#include <AvUtils/avString.h>
#include <AvUtils/filesystem/avFile.h>
#include <stdarg.h>

void avStringPrintfToFile(AvFile file, AvString format, ...);
void avStringPrintfToFileVA(AvFile file, AvString format, va_list args);

void avStringPrintfToBuffer(char* buffer, uint32 bufferSize, AvString format, ...);
void avStringPrintfToBufferVA(char* buffer, uint32 bufferSize, AvString format, va_list args) ;

void avStringPrintfToFileDescriptor(AvFileDescriptor out, AvString format, ...);
void avStringPrintfToFileDescriptorVA(AvFileDescriptor out, AvString format, va_list args);

void avStringPrintf(AvString format, ...);
void avStringPrintfVA(AvString format, va_list args) ;


C_SYMBOLS_END
#endif//__AV_PRINTF__