#ifndef __AV_DIRECTORY__
#define __AV_DIRECTORY__
#include "../avDefinitions.h"
C_SYMBOLS_START

#include "../avTypes.h"
#include "../avString.h"
#include "../avDataStructures.h"
#include "avFile.h"


typedef enum AvDirectoryEntryType{
	AV_DIRECTORY_ENTRY_TYPE_FILE,
	AV_DIRECTORY_ENTRY_TYPE_DIRECTORY,
}AvDirectoryEntryType;

typedef struct AvDirectoryTree_T* AvDirectoryTree;
typedef struct AvDirectoryEntry_T* AvDirectoryEntry;
typedef struct AvDirectoryEntry_T {
	AvDirectoryEntryType type;
	AvDirectoryTree tree;
	AvString path;
	union {
		struct directory{
			bool32 explored;
			AV_DS(AvDynamicArray, AvDirectoryEntry) contents;
		} directory;
		struct file {
			AvFile fileHandle;
		} file;
	};
} AvDirectoryEntry_T;



bool32 avDirectoryTreeCreate(AvString rootDir, AvDirectoryTree* tree);

AvDirectoryEntry avDirectoryTreeGetRootDir(AvDirectoryTree tree);

void avDirectoryExplore(AvDirectoryEntry dir);

#define AV_DIRECTORY_UNEXPLORED ((uint32)-1)

uint32 avDirectoryGetContentCount(AvDirectoryEntry dir);
void avDirectoryGetContents(AvDirectoryEntry* contents, AvDirectoryEntry dir);

void avDirectoryTreeDestroy(AvDirectoryTree tree);




C_SYMBOLS_END
#endif//__AV_DIRECTORY__