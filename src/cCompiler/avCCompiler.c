#include "avCCompiler.h"
#include "avCParser.h"

AvAllocator allocator;

void avCCompilerCompile(AvString str){
    avAllocatorCreate(0, AV_ALLOCATOR_TYPE_DYNAMIC, &allocator);

    struct TranslationUnit translationUnit = parseTranslationUnit(str);
    (void) translationUnit;

    avAllocatorDestroy(&allocator);
}