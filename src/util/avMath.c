#include <AvUtils/avMath.h>


uint32 nextPow2(uint32 x) { 	
	return x == 1 ? 1 : 1<<(32-__builtin_clz(x-1)); 
}

uint64 nextPow2L(uint64 x){
	return x == 1 ? 1ULL : 1ULL<<(64U-__builtin_clz(x-1U)); 
}