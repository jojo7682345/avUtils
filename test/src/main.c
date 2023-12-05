#include <AvUtils/avMemory.h>
#include <AvUtils/avThreading.h>
#define AV_DYNAMIC_ARRAY_EXPOSE_MEMORY_LAYOUT
#include <AvUtils/avDataStructures.h>
#include <AvUtils/avLogging.h>
#include <AvUtils/avString.h>
#include <AvUtils/string/avPath.h>
#include <AvUtils/avFileSystem.h>
#include <AvUtils/avProcess.h>
#include <AvUtils/avBuilder.h>


#include <stdio.h>

void testQueue() {
	AvQueue queue;
	avQueueCreate(sizeof(int), 2, &queue);


	printf("availableSlots: %li\nfilledSlots: %li\n", avQueueGetRemainingSpace(queue), avQueueGetOccupiedSpace(queue));

	for (int i = 0; i < avQueueGetSize(queue); i++) {
		avQueuePush(&i, queue);
	}

	if (avQueueIsFull(queue)) {
		printf("queue filled\n");
	}
	printf("availableSlots: %li\nfilledSlots: %li\n", avQueueGetRemainingSpace(queue), avQueueGetOccupiedSpace(queue));

	for (int i = 0; i < avQueueGetSize(queue); i++) {
		int value;
		avQueuePull(&value, queue);
		printf("%i\n", value);
	}

	if (avQueueIsEmpty(queue)) {
		printf("queue emptied\n");
	} else {
		printf("queue not emptied\n");
	}

	printf("availableSlots: %li\nfilledSlots: %li\n", avQueueGetRemainingSpace(queue), avQueueGetOccupiedSpace(queue));

	for (int i = 0; i < avQueueGetSize(queue); i++) {
		avQueuePush(&i, queue);
	}

	if (avQueueIsFull(queue)) {
		printf("queue filled\n");
	}
	printf("availableSlots: %li\nfilledSlots: %li\n", avQueueGetRemainingSpace(queue), avQueueGetOccupiedSpace(queue));

	for (int i = 0; i < avQueueGetSize(queue); i++) {
		int value;
		if (!avQueuePull(&value, queue)) {
			break;
		}
		printf("%i\n", value);
	}

	avQueueDestroy(queue);
}

uint32 threadFuncA(void* data, uint64 dataSize) {
	printf("Thread A entry\n");
	avThreadSleep(100);
	printf("Thread A exit\n");
	return 0;
}

uint32 threadFuncB(void* data, uint64 dataSize) {
	printf("Thread B entry\n");
	avThreadSleep(100);
	printf("Thread B exit\n");
	return 0;
}

void testThread() {
	AvThread threadA;
	AvThread threadB;
	avThreadCreate((AvThreadEntry)&threadFuncA, &threadA);
	avThreadCreate((AvThreadEntry)&threadFuncB, &threadB);

	avThreadStart(nullptr, 0, threadA);
	avThreadStart(nullptr, 0, threadB);

	avThreadJoin(threadA);
	avThreadJoin(threadB);

	avThreadDestroy(threadA);
	avThreadDestroy(threadB);
}

uint32 mutexFunc(void* data, uint64 dataSize) {
	AvMutex mutex = (AvMutex)data;

	avThreadSleep(10);

	avMutexLock(mutex);
	printf("threadB\n");
	avMutexUnlock(mutex);
	return 0;
}

void testMutex() {
	AvThread thread;
	AvMutex mutex;
	avThreadCreate((AvThreadEntry)&mutexFunc, &thread);
	avMutexCreate(&mutex);

	avThreadStart(mutex, sizeof(AvMutex), thread);

	avMutexLock(mutex);

	avThreadSleep(100);
	printf("threadA\n");

	avMutexUnlock(mutex);

	avThreadJoin(thread);

	avMutexDestroy(mutex);
	avThreadDestroy(thread);
}

void testDynamicArray() {

	AvDynamicArray arr;
	avDynamicArrayCreate(4, sizeof(uint32), &arr);

	printf("dynamic array data:\n\tcount:%i\n\tcapacity:%i\n", avDynamicArrayGetSize(arr), avDynamicArrayGetCapacity(arr));

	uint32 data = 1;

	avDynamicArrayAdd(&data, arr);

	printf("dynamic array data:\n\tcount:%i\n\tcapacity:%i\n", avDynamicArrayGetSize(arr), avDynamicArrayGetCapacity(arr));

	for (uint32 i = 0; i < 32; i++) {
		avDynamicArrayAdd(&i, arr);
	}

	printf("dynamic array data:\n\tcount:%i\n\tcapacity:%i\n", avDynamicArrayGetSize(arr), avDynamicArrayGetCapacity(arr));

	for (uint i = 0; i < avDynamicArrayGetSize(arr); i++) {
		uint32 data = 0;
		avDynamicArrayRead(&data, i, arr);
		printf("index %i = %i\n", i, data);
	}

	avDynamicArrayReserve(32, arr);
	uint32 doesNotWrite = 40;
	avDynamicArrayWrite(&doesNotWrite, 31, arr);
	printf("dynamic array data:\n\tcount:%i\n\tcapacity:%i\n", avDynamicArrayGetSize(arr), avDynamicArrayGetCapacity(arr));

	avDynamicArrayTrim(arr);

	printf("dynamic array data:\n\tcount:%i\n\tcapacity:%i\n", avDynamicArrayGetSize(arr), avDynamicArrayGetCapacity(arr));

	avDynamicArrayRemove(1, arr);

	for (uint i = 0; i < avDynamicArrayGetSize(arr); i++) {
		uint32 data = 0;
		avDynamicArrayRead(&data, i, arr);
		printf("index %i = %i\n", i, data);
	}
	printf("dynamic array data:\n\tcount:%i\n\tcapacity:%i\n", avDynamicArrayGetSize(arr), avDynamicArrayGetCapacity(arr));

	uint32 number = 54;
	avDynamicArrayAdd(&number, arr);

	avDynamicArrayTrim(arr);

	avDynamicArrayReserve(20, arr);

	printf("printing memory layout\n");
	uint32 memCount = avDynamicArrayGetPageCount(arr);
	for (uint i = 0; i < memCount; i++) {
		printf("|");
		uint32 size = avDynamicArrayGetPageSize(i, arr);
		uint32 capacity = avDynamicArrayGetPageCapacity(i, arr);
		for (uint j = 0; j < capacity; j++) {
			if (j < size) {
				printf("#");
			} else {
				printf("-");
			}
		}
		printf("|\n");
	}

	uint32* buffer = avCallocate(avDynamicArrayGetSize(arr), sizeof(uint32), "");
	avDynamicArrayReadRange(buffer, avDynamicArrayGetSize(arr), 0, sizeof(uint32), 0, arr);
	for (uint i = 0; i < avDynamicArrayGetSize(arr); i++) {
		printf("%u\n", buffer[i]);
	}

	avDynamicArrayDestroy(arr);
}


void testString() {

	avStringDebugContextStart;

	AvString str = AV_CSTR("test string_ ");

	strOffset offset = avStringFindFirstOccranceOfChar(str, ' ');

	printf("char ' ' offset of %lu\n", offset);

	avStringReplaceChar(&str, ' ', '_');
	avStringPrint(str);
	printf("\n");

	AvString replacedString = AV_EMPTY;
	avStringReplace(&replacedString, str, AV_CSTR("_"), AV_CSTR(" insert "));
	avStringFree(&str);
	avStringPrint(str);



	printf("\n");


	AvString splitString = { 0 };
	avStringClone(&splitString, AV_CSTR(":split string at:other string"));
	AV_DS(AvArray, AvString) strings = AV_EMPTY;

	avStringSplitOnChar(&strings, ':', splitString);
	avStringFree(&splitString);

	avArrayForEachElement(AvString, element, i, &strings,
		avStringPrintln(element);
	);

	avArrayFree(&strings);


	avStringDebugContextEnd;
}

void testPath(const char* location) {
	avStringDebugContextStart;
	AvPath path = AV_EMPTY;
	avPathOpen(AV_CSTR(location), &path);

	avStringPrint(path.path);
	if (path.type == AV_PATH_TYPE_DIRECTORY) {
		printf(" is a directory\n");
		avPathGetDirectoryContents(&path);

	} else if (path.type == AV_PATH_TYPE_FILE) {
		printf(" is a file\n");
	} else {
		printf(" is invalid\n");
	}
	for (uint32 i = 0; i < path.directory.contentCount; i++) {
		//avStringPrintData(path.directory.content[i].path);
		avStringPrintln(path.directory.content[i].path);
	}

	avPathClose(&path);
	avStringDebugContextEnd;
}

void testFile() {

	avStringDebugContextStart;

	AvString filePath = AV_EMPTY;
	avFileBuildPathVAR("README.md", &filePath, AV_ROOT_DIR, "SDK_CCR", "avUtils");

	AvFile file;
	avFileHandleCreate(filePath, &file);

	AvFileNameProperties* nameProperties = avFileHandleGetFileNameProperties(file);
	avStringPrintln(nameProperties->fileFullPath);
	avStringPrintln(nameProperties->fileName);
	avStringPrintln(nameProperties->filePath);
	avStringPrintln(nameProperties->fileExtension);
	avStringPrintln(nameProperties->fileNameWithoutExtension);

	if (!avFileExists(file)) {
		printf("file %s does not exits\n", nameProperties->fileFullPath.chrs);
	} else {
		printf("file %s exists\n", nameProperties->fileFullPath.chrs);
	}

	AvDateTime created = avFileGetAccessedTime(file);

	AvString timeString = { 0 };
	avTimeConvertToString(created, &timeString, AV_DATE_FORMAT_SS_MM_HH_DD_MM_YYYY);
	printf("file was created at ");
	avStringPrintLn(timeString);
	avStringFree(&timeString);

	uint64 size = avFileGetSize(file);
	printf("file size : %lu\n", size);

	avFileOpen(file, (AvFileOpenOptions) { .openMode = AV_FILE_OPEN_READ, .binary = false, .update = false });

	char* buffer = avCallocate(1, size + 1, "read buffer allocation");
	buffer[size] = '\0';

	avFileRead(buffer, size, file);

	printf(buffer);

	avFileClose(file);

	avFileHandleDestroy(file);
	avStringFree(&filePath);

	avStringDebugContextEnd;
}

void testProcess() {
	AvProcess gcc;
	avProcessCreate("echo", NULL, NULL, &gcc);

	const char* arg[] = {
		"test"
	};
	if (!avProcessStart(sizeof(arg) / sizeof(char*), arg, gcc)) {
		printf("failed to start gcc\n");
	}
	uint32 retCode = 0;
	if (!avProcessWaitForTermination(&retCode, gcc)) {
		printf("failed to return from gcc\n");
	}



	avProcessDestroy(gcc);

}

void testBuild(){
	if(avCompileBuildFile(AV_CSTR("./avUtilsBuild.cbuild"))!=AV_PARSE_RESULT_SUCCESS){
		printf("compiling failed\n");
	}else{
		printf("compiled successfully\n");
	}
}

int main() {
	avStringDebugContextStart;

	testDynamicArray();
	testQueue();
	testThread();
	testMutex();
	testFile();
	testString();
	testPath("/");

	testBuild();
	//testProcess();
	printf("test completed\n");
	avStringDebugContextEnd;
	return 0;

}