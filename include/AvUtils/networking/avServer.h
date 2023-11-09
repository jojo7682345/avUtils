#ifndef __AV_SERVER__
#define __AV_SERVER__

#include "../avTypes.h"

typedef struct AvServer_T* AvServer;

bool32 avServerCreate(uint16 port, AvServer* server);

#endif//__AV_SERVER__