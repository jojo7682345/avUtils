#include <avMemory.h>
#include <avThread.h>
#include <avQueue.h>
#include <stdio.h>


int main() {

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


	return 0;

}