#include <AvUtils/logging/avAssert.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

//TODO implement propper logging

#if defined(__has_builtin) && !defined(__ibmxl__)
#	if __has_builtin(__builtin_debugtrap)
#		define debug_break() __builtin_debugtrap()
#	elif __has_builtin(__debugbreak)
#		define debug_break() __debugbreak()
#	endif
#endif

// If not setup, try the old way.
#if !defined(debug_break)
#	if defined(__clang__) || defined(__gcc__)
/** @brief Causes a debug breakpoint to be hit. */
#		define debug_break() __builtin_trap()
#	elif defined(_MSC_VER)
#		include <intrin.h>
/** @brief Causes a debug breakpoint to be hit. */
#		define debug_break() __debugbreak()
#	else
// Fall back to x86/x86_64
#		define debug_break()  __asm__ volatile (" int3 ")
#	endif
#endif

void avAssertFailed(bool32 condition, const char* expression, const char* message, uint64 line, const char* function, const char* file) {
	printf("assert failed:\n\t%s\n\tfunc:\t%s\n\tline:\t%" PRIu64 "\n\tfile:\t%s\n\tcode:\t(%s:%"PRIu64")\n\t%s\n", expression, function, line, file, file, line, message);
	debug_break();
	exit(1);
}
