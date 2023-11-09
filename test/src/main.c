#include <AvUtils/avMemory.h>
#include <AvUtils/avThreading.h>
#include <AvUtils/avDataStructures.h>
#include <stdio.h>
#include <AvUtils/avLogging.h>

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

int main() {

	testQueue();
	testThread();
	testMutex();
	return 0;

}