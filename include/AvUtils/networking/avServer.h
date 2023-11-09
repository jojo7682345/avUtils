#ifndef __AV_SERVER__
#define __AV_SERVER__

#include "../avTypes.h"
#include "./avSocket.h"

typedef struct AvServer_T* AvServer;

bool32 avServerCreate(uint16 port, uint32 maxClients, uint32 networkOptions, AvServer* server);

void avServerDestroy(AvServer server); 



#endif//__AV_SERVER__