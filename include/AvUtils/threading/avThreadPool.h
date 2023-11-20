#ifndef __AV_THREAD_POOL__
#define __AV_TRHEAD_POOL__

#include "../avTypes.h"

typedef struct AvThreadPool_T* AvThreadPool;

void avThreadPoolCreate(uint32 threadCount, AvThreadPool* pool);

void avThreadPoolDestroy(AvThreadPool pool);


#endif//__AV_TRHEAD_POOL__