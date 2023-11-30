#ifndef __AV_DIRECTORY__
#define __AV_DIRECTORY__
#include "../avDefinitions.h"
C_SYMBOLS_START

#include "../string/avPath.h"
#include "../avTypes.h"
#include "../avString.h"
#include "avFile.h"

typedef enum AvDirectoryContentType {
	AV_DIRECTORY_CONTENT_TYPE_INVALID,
	AV_DIRECTORY_CONTENT_TYPE_DIRECTORY,
	AV_DIRECTORY_CONTENT_TYPE_FILE,
} AvDirectoryContentType;

typedef struct AvDirectory_T* AvDirectory;
typedef struct AvDirectoryContent {
	AvDirectoryContentType type;
	union {
		AvFile file;
		AvDirectory directory;
	};
} AvDirectoryContent;
 

bool32 avDirectoryOpen(AvPath path, AvDirectory* dir);

uint32 avDirectoryGetContentCount(AvDirectory dir);
AvDirectoryContentType avDirectoryGetContentType(uint32 index, AvDirectory dir);
AvFile avDirectoryOpenFile(uint32 index, AvDirectory dir);
AvDirectory avDirectoryOpenSubfolder(uint32 index, AvDirectory dir);

uint32 avDirectoryRefresh(AvDirectory dir);

void avDirectoryGetPathStr(AvStringRef str, AvDirectory dir);

void avDirectoryClose(AvDirectory* dir);

C_SYMBOLS_END
#endif//__AV_DIRECTORY__