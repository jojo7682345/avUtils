#include <avMemory.h>


int main() {

	byte* data = avAllocate(300, "");

	avFree(data);

	return 0;

}