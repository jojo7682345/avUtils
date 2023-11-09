#ifndef __AV_SOCKET__
#define __AV_SOCKET__

#include "../avTypes.h"

typedef enum AvConnectionType {
    AV_CONNECTION_TYPE_TCP = 0b00,
    AV_CONNECTION_TYPE_UDP = 0b01,
} AvConnectionType;

typedef enum AvNetworkType {
    AV_NETWORK_TYPE_IPV4 = 0b00,
    AV_NETWORK_TYPE_IPV6 = 0b10,
} AvNetworkType;

#define AV_DEFAULT_NETWORK_OPTIONS (AV_CONNECTION_TYPE_TCP | AV_NETWORK_TYPE_IPV4)

bool32 avSocketCreate(uint32 networkOptions, uint16 port, AvSocket* socket);
void avSocketDestroy(AvSocket socket);

typedef struct AvSocket_T* AvSocket;


#endif//__AV_SOCKET__