#include <avMemory.h>
#include <avThread.h>

uint task(byte* buffer, uint64 bufferSize) {

	return 0;
};

int main() {

	AvThread thread;
	avCreateThread((AvThreadEntry) &task, &thread);

	avStartThread(nullptr, 0, thread);

	avJoinThread(thread);

	avDestroyThread(thread);

	return 0;

}