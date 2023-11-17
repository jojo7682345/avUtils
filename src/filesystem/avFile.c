#include <AvUtils/filesystem/AvFile.h>
#include <AvUtils/avMemory.h>

#define AV_DYNAMIC_ARRAY_EXPOSE_MEMORY_LAYOUT
#include <AvUtils/dataStructures/avDynamicArray.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <stddef.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <io.h>
#define F_OK 0
#define access _access
#define stat _stat
#else
#include <unistd.h>

#endif

typedef struct AvFile_T {
	AvFileNameProperties nameProperties;
	AvFileStatus status;
	FILE* filehandle;
#ifdef _WIN32

#else

#endif
} AvFile_T;

void avFileBuildPathVAR_(const char* fileName, AvAllocatedString* filePath, ...){

	AvDynamicArray arr;
	avDynamicArrayCreate(0, sizeof(AvString), &arr);
	va_list args; 
	va_start(args, filePath);
	char* arg;
	while(arg = va_arg(args, char*)){
		AvString currentArg = { 0 };
		currentArg.chrs = (char*)arg;
		currentArg.len = avStringLength(arg);
		avDynamicArrayAdd(&currentArg, arr);
	}
	va_end(args);
	avDynamicArrayMakeContiguous(arr);

	avFileBuildPathARR(fileName, filePath, avDynamicArrayGetSize(arr), avDynamicArrayGetPageDataPtr(0, arr));

	avDynamicArrayDestroy(arr);
}

void avFileBuildPathARR(const char* fileName, AvAllocatedString* filePath, uint32 directoryCount, AvString directories[]){

	AvString fileNameStr = AV_STR((char*)fileName);
	
	uint64 length = 0;
	for(uint i = 0; i < directoryCount; i++){
		length += directories[i].len + 1;
	}
	length += fileNameStr.len;
	AvPersistentStringMemory strMem;
	avStringMemoryCreatePersistent(&strMem);
	avStringMemoryAllocate(length, strMem);
	
	uint64 offset = 0;
	for(uint32 i = 0; i < directoryCount; i++){
		avStringMemoryStore(directories[i], offset, directories[i].len, strMem);
		offset += directories[i].len;
		avStringMemoryStore(AV_STRL("/", 1), offset++, 1, strMem);
	}
	avStringMemoryStore(fileNameStr, offset, fileNameStr.len, strMem);
	avStringMemoryCreateString(0, AV_STRING_FULL_LENGTH, filePath, strMem);

}

void avFileHandleCreate(AvString filePath, AvFile* file) {

	(*file) = avCallocate(1, sizeof(AvFile_T), "allocating file handle");
	(*file)->status = AV_FILE_STATUS_CLOSED;
	AvString filePathStr = filePath;

	AvFileNameProperties* nameProperties = &(*file)->nameProperties;
	avStringMemoryAllocate(filePathStr.len, &nameProperties->fileStr);
	avStringMemoryStore(filePathStr, 0, AV_STRING_FULL_LENGTH, &nameProperties->fileStr);
	avStringMemoryCreateString(0, AV_STRING_FULL_LENGTH, &nameProperties->fileFullPath, &nameProperties->fileStr);

	avStringReplaceChar(nameProperties->fileFullPath.str, '\\', '/');	

	strOffset filePathLen = avStringFindLastOccuranceOfChar(nameProperties->fileFullPath.str, '/'); 
	if(filePathLen++ == AV_STRING_NULL){
		filePathLen = 0;	
	}else{
		avStringMemoryCreateString(0, filePathLen, &nameProperties->filePath, &nameProperties->fileStr);
	}

	avStringMemoryCreateString(filePathLen, AV_STRING_FULL_LENGTH, &nameProperties->fileName, &nameProperties->fileStr);

	strOffset fileExtOffset = avStringFindLastOccuranceOfChar(nameProperties->fileName.str, '.');
	if(fileExtOffset == AV_STRING_NULL){
		fileExtOffset = 0;
	}else{
		avStringMemoryCreateString(filePathLen + fileExtOffset, AV_STRING_FULL_LENGTH, &nameProperties->fileExtension, &nameProperties->fileStr);
	}
	avStringMemoryCreateString(filePathLen, fileExtOffset, &nameProperties->fileNameWithoutExtension, &nameProperties->fileStr);
}

AvFileNameProperties* avFileHandleGetFileNameProperties(AvFile file){
	return &file->nameProperties;
}

bool32 avFileExists(AvFile file) {
	if(access(file->nameProperties.fileFullPath.str.chrs, F_OK)==0){
		return true;
	}else{
		return false;
	}
}

bool32 avFileOpen(AvFile file, AvFileOpenOptions mode) {
	char openMode[4] = {0};
	uint wi = 1;
	(mode.openMode == AV_FILE_OPEN_READ) ? openMode[0]='r' : 0;
	(mode.openMode == AV_FILE_OPEN_WRITE) ? openMode[0]='w' : 0;
	(mode.openMode == AV_FILE_OPEN_APPEND) ? openMode[0]='a' : 0;
	(mode.update == true) ? openMode[wi++]='+' : 0;
	(mode.binary = true) ? openMode[wi++]='b' : 0;

	file->filehandle = fopen(file->nameProperties.fileFullPath.str.chrs, openMode);
	if(file->filehandle==NULL){
		file->status = AV_FILE_STATUS_CLOSED;
		return false;
	}
}

AvFileStatus avFileGetStatus(AvFile file){
	return file->status;
}


#define statProp(prop) (offsetof(struct stat, prop))
static uint64 getFileUint64Stat(AvFile file, uint64 offset){
	struct stat stats = {0};
	stat(file->nameProperties.fileFullPath.str.chrs, &stats);
	return *(uint64*)(((byte*)&stats)+offset);
}

static AvDateTime getFileTimeStat(AvFile file, uint64 offset){
	struct stat stats = {0};
	stat(file->nameProperties.fileFullPath.str.chrs, &stats);
	time_t time = *(time_t*)(((byte*)&stats)+offset);
	struct tm* dateTime = gmtime(&time);
	AvDateTime result = {0}; 
	result.second = (uint8) dateTime->tm_sec;
	result.minute = (uint8) dateTime->tm_min;
	result.hour = (uint8) dateTime->tm_hour;
	result.day = (uint8) dateTime->tm_mday;
	result.month = (uint8) dateTime->tm_mon;
	result.year = (uint8) dateTime->tm_year+1900;
	return result;
}

AvDateTime avFileGetCreationTime(AvFile file){
	return getFileTimeStat(file,statProp(st_ctime));
}

AvDateTime avFileGetAccessedTime(AvFile file){
	return getFileTimeStat(file,statProp(st_atime));
}

AvDateTime avFileGetModifiedTime(AvFile file){
	return getFileTimeStat(file,statProp(st_mtime));
}

uint64 avFileGetSize(AvFile file){
	return getFileUint64Stat(file, statProp(st_size));
}

uint64 avFileRead(void* dst, uint64 size, AvFile file){

	if((file->status & AV_FILE_STATUS_OPEN_READ | AV_FILE_STATUS_OPEN_UPDATE) == 0){
		return 0;
	}
	return fread(dst, size, 1, file->filehandle);
}

uint64 avFileWrite(void* src, uint64 size, AvFile file){
	if((file->status & AV_FILE_STATUS_OPEN_WRITE | AV_FILE_STATUS_OPEN_UPDATE) == 0){
		return 0;
	}
	return fwrite(src, size, 1, file->filehandle);
}


void avFileClose(AvFile file) {
	if(file->status <= AV_FILE_STATUS_CLOSED){
		return;
	}
	if(file->filehandle){
		fclose(file->filehandle);
		file->filehandle = nullptr;
		file->status = AV_FILE_STATUS_CLOSED;
	}
}

void avFileHandleDestroy(AvFile file) {
	avStringMemoryFree(&file->nameProperties.fileStr);
	file->status = AV_FILE_STATUS_UNKNOWN;
	avFree(file);
}

#ifdef _WIN32

#else

#endif
