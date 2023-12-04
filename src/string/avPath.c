#include <AvUtils/string/avPath.h>
#include <AvUtils/string/avChar.h>
#include <AvUtils/avMemory.h>
#include <AvUtils/avLogging.h>
#define AV_DYNAMIC_ARRAY_EXPOSE_MEMORY_LAYOUT
#include <AvUtils/dataStructures/avDynamicArray.h>
#include <string.h>
#include <stdio.h>

#ifdef _WIN32
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif
