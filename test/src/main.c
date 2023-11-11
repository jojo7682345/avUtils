#include <AvUtils/avMemory.h>
#include <AvUtils/avThreading.h>
#define AV_DYNAMIC_ARRAY_EXPOSE_MEMORY_LAYOUT
#include <AvUtils/avDataStructures.h>
#include <AvUtils/avLogging.h>

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

	avThreadSleep(100);
	
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

	avThreadSleep(1000);
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
		uint32 size = avDynamicArrayGetPageSize(i,arr);
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

	avDynamicArrayDestroy(arr);
}

int main() {

	testDynamicArray();
	testQueue();
	testThread();
	testMutex();
	return 0;

}