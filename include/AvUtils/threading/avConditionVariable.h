#ifndef __AV_CONDITION_VARIABLE__
#define __AV_CONDITION_VARIABLE__

#include "../avDefinitions.h"
C_SYMBOLS_START

#include "../avTypes.h"
#include "avMutex.h"

typedef struct AvConditionVariable_T* AvConditionVariable;

// create / destroy
bool32 avConditionVariableCreate(AvConditionVariable* condition);
void avConditionVariableDestroy(AvConditionVariable condition);

// wait (must be called with locked mutex)
void avConditionVariableWait(AvConditionVariable condition, AvMutex mutex);

// timed wait (returns false on timeout)
bool32 avConditionVariableTimedWait(AvConditionVariable condition, AvMutex mutex, uint32 timeoutMs);

// wake one thread
void avConditionVariableSignal(AvConditionVariable condition);

// wake all threads
void avConditionVariableBroadcast(AvConditionVariable condition);

C_SYMBOLS_END
#endif // __AV_CONDITION_VARIABLE__