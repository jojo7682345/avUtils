#ifndef __AV_FILE__
#define __AV_FILE__

#include "../avTypes.h"
#include "../avString.h"
#include "../util/avTime.h"

#define AV_ROOT_DIR ""
#define AV_CURRENT_DIR "."
#define AV_PARENT_DIR ".."

typedef struct AvFile_T* AvFile;

typedef enum {
	AV_FILE_OK = 0,
	AV_FILE_ERROR = 1,
	AV_FILE_NOT_FOUND = 2,
	AV_FILE_UNABLE_TO_OPEN = 3,
}AvFileErrorCode;

typedef enum AvFileStatus {
	AV_FILE_STATUS_UNKNOWN = 0,
	AV_FILE_STATUS_CLOSED = 1,
	AV_FILE_STATUS_OPEN_READ = 0x2,
	AV_FILE_STATUS_OPEN_WRITE = 0x4,
	AV_FILE_STATUS_OPEN_UPDATE = 0x8,
} AvFileStatus;

typedef enum AvFileOpenMode{
	AV_FILE_OPEN_WRITE = 0,
	AV_FILE_OPEN_READ = 0x1,
	AV_FILE_OPEN_APPEND = 0x2,
}AvFileOpenMode;

typedef struct AvFileOpenOptions{
	byte openMode;
	bool8 update;
	bool8 binary;
} AvFileOpenOptions;



#define AV_FILE_MAX_FILENAME_LENGTH 512
#define AV_FILE_MAX_PATH_LENGTH 512
#define AV_FILE_PADDING_LENGTH 3

typedef struct AvFileNameProperties {
	AvStringMemory fileStr;
	AvAllocatedString fileFullPath;
	AvAllocatedString filePath;
	AvAllocatedString fileName;
	AvAllocatedString fileNameWithoutExtension;
	AvAllocatedString fileExtension;
}AvFileNameProperties;

#define avFileBuildPathVAR(fileName, filePath, ...) avFileBuildPathVAR_(fileName, filePath, __VA_ARGS__, NULL)
void avFileBuildPathVAR_(const char* fileName, AvAllocatedString* filePath, ...);
void avFileBuildPathARR(const char* fileName, AvAllocatedString* filePath, uint32 directoryCount, AvString direcotries[]);

void avFileHandleCreate(AvString filePath, AvFile* file);

AvFileNameProperties* avFileHandleGetFileNameProperties(AvFile file);

bool32 avFileExists(AvFile file);
uint64 avFileGetSize(AvFile file);
AvDateTime avFileGetCreationTime(AvFile file);
AvDateTime avFileGetAccessedTime(AvFile file);
AvDateTime avFileGetModifiedTime(AvFile file);
AvFileStatus avFileGetStatus(AvFile file);

bool32 avFileOpen(AvFile file, AvFileOpenOptions mode);

uint64 avFileRead(void* dst, uint64 size, AvFile file);
uint64 afFileWrite(void* src, uint64 size, AvFile file);


void avFileClose(AvFile file);

void avFileHandleDestroy(AvFile file);

#endif//__AV_FILE__
