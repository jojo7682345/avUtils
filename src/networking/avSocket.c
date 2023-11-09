#include <AvUtils/networking/avSocket.h>

#ifdef _WIN32

#else
#include <sys/socket.h>
#include <sys/types.h>
#endif


typedef struct AvSocket_T{ 
    

#ifdef _WIN32

#else
    int socketFD;
#endif
} AvSocket_T;

