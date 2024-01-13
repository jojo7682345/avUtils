#include "avCCompiler.h"
#include "avCParser.h"

AvAllocator allocator;

void avCCompilerCompile(AvString str){
    avAllocatorCreate(0, AV_ALLOCATOR_TYPE_DYNAMIC, &allocator);

    TokenList tokens = tokenizeSource(str);
    struct TranslationUnit translationUnit = parseTokenList(tokens);
    (void) translationUnit;

    avAllocatorDestroy(&allocator);
}