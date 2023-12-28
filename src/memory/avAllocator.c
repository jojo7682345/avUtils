#include <AvUtils/memory/avAllocator.h>
#include <AvUtils/avLogging.h>

#define ALLOC_FUNC(type, func, ...) av##type##Allocator##func (__VA_ARGS__  ( Av##type##Allocator* ) allocator);
#define ALLOC_FUNC_CASE(TYPE, type, func, op, ...) case AV_ALLOCATOR_TYPE_##TYPE: \
    op ALLOC_FUNC (type, func, __VA_ARGS__)\
    break;\

#define ALLOC_FUNCS(func,op, ...) switch (allocator->type) {\
        ALLOC_FUNC_CASE(DYNAMIC, Dynamic, func, op, __VA_ARGS__)\
        ALLOC_FUNC_CASE(LINEAR, Linear, func, op, __VA_ARGS__)\
        default: avAssert(0, "invalid allocator type"); break;\
    }\

void avAllocatorCreate(uint64 size, AvAllocatorType type, AvAllocator* allocator) {
    allocator->type = type;
    ALLOC_FUNCS(Create, , size,);
}
void* avAllocatorAllocate(uint64 size, AvAllocator* allocator) {
    ALLOC_FUNCS(Allocate, return, size,);
    return nullptr;
}
void avAllocatorReset(AvAllocator* allocator) {
    ALLOC_FUNCS(Reset, ;,);
}
void avAllocatorDestroy(AvAllocator* allocator) {
    ALLOC_FUNCS(Destroy, ;,);
}