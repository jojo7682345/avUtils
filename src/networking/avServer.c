#include <AvUtils/networking/avServer.h>

#include <AvUtils/avMemory.h>
#include <AvUtils/avDataStructures.h>
#include <AvUtils/avLogging.h>
#include <AvUtils/avThreading.h>

#ifdef _WIN32
#include <winsock.h>
#else

#endif


typedef struct AvServer_T {

	uint32 maxClients;
	AvThread* listenThreads;
	AvSocket* sockets;
#ifdef _WIN32

#else

#endif
} AvServer_T;

typedef struct Connection {
	AvServer server;
	uint32 connectionID;
}Connection;

static int listenForConnection(void* data, uint64 dataSize){
	AvServer server = ((Connection*)data)->server;
	uint32 connectionID = ((Connection*)data)->connectionID;


}

bool32 avServerCreate(uint16 port, uint32 maxClients, uint32 networkOptions, AvServer* server) {
	(*server) = avCallocate(1,sizeof(AvServer_T), "allocating server handle");
	(*server)->maxClients = maxClients;
	(*server)->listenThreads = avCallocate(maxClients, sizeof(AvThread), "allocating listen threads");
	(*server)->sockets = avCallocate(maxClients, sizeof(AvSocket), "allocating sockets");

	for(uint32 i = 0; i < maxClients; i++){
		avThreadCreate(&listenForConnection, &(*server)->listenThreads[i]);
	}
	return 0;
}

void avServerStart()

void avServerDestroy(AvServer server){
	avFree(server->listenThreads);
	avFree(server->sockets);
	avFree(server);
}




