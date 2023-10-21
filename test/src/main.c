#include <avMemory.h>
#include <avThread.h>
#include <stdio.h>

uint taskA(byte* buffer, uint64 bufferSize) {

	printf("hello from thread A\n");
	for (uint i = 0; i < (uint)-1; i++) {
		// loop
	}
	printf("Goodbye from thread A\n");
	return 0;
};

uint taskB(byte* buffer, uint64 bufferSize) {

	printf("hello from thread B\n");
	for (uint i = 0; i < (uint)-1; i++) {
		// loop
	}
	printf("Goodbye from thread B\n");
	return 0;
};

int main() {

	AvThread threadA;
	AvThread threadB;
	avCreateThread((AvThreadEntry) &taskA, &threadA);
	avCreateThread((AvThreadEntry) &taskB, &threadB);

	avStartThread(nullptr, 0, threadA);
	avStartThread(nullptr, 0, threadB);

	uint ret = avJoinThread(threadA);
	avJoinThread(threadB);

	printf("%i\n", ret);

	avDestroyThread(threadA);
	avDestroyThread(threadB);


	return 0;

}