#include <avUtils/networking/avServer.h>

#include <avUtils/avMemory.h>
#include <avUtils/avDataStructures.h>
#include <avUtils/avLogging.h>

#ifdef _WIN32
#include <winsock.h>

typedef struct AvServer_T {

} AvServer_T;


bool32 avServerCreate(uint16 port, AvServer* server) {



	return 0;
}

#else

typedef struct AvServer_T {

} AvServer_T;

#endif


