#define AV_DYNAMIC_ARRAY_EXPOSE_MEMORY_LAYOUT
#include "../avInterpreter.h"
#include "../avTokenizer.h"
#include <AvUtils/avMemory.h>
#include <AvUtils/avLogging.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#define CASSERT(predicate, file) _impl_CASSERT_LINE(predicate,__LINE__,file)

#define _impl_PASTE(a,b) a##b
#define _impl_CASSERT_LINE(predicate, line, file) \
    typedef char _impl_PASTE(assertion_failed_##file##_,line)[2*!!(predicate)-1];

typedef enum LengthSpefifier {
    LENGTH_0b = 0x0,
    LENGTH_1b = 0x1,
    LENGTH_2b = 0x2,
    LENGTH_4b = 0x4,
    LENGHT_8b = 0x8,
}LengthSpefifier;

typedef enum SignSpecifier {
    SPECIFIER_UNSIGNED = 0x0,
    SPECIFIER_SIGNED = 0x1,
} SignSpecifier;

typedef enum AccessModifier {
    ACCESS_NORMAL = 0x0,
    ACCESS_CONST = 0x1,
    ACCESS_VOLATILE = 0x2,
    ACCESS_RESTRICT = 0x4,
} AccessModifier;

typedef union FloatFlags {
    struct {
        LengthSpefifier length : 4;

    };
    uint32 bits;
}FloatFlags;
CASSERT(sizeof(FloatFlags) == sizeof(((FloatFlags*)0)->bits), typeParsing);

typedef union IntFlags {
    struct {

    };
    uint32 bits;
}IntFlags;
CASSERT(sizeof(IntFlags) == sizeof(((IntFlags*)0)->bits), typeParsing)

typedef enum TypeType {
    TYPE_NONE = 0x0,
    TYPE_VOID = 0x1,
    TYPE_INTEGER = 0x2,
    TYPE_FLOAT = 0x4,
    TYPE_ENUM = 0x8,
    TYPE_UNION = 0x10,
    TYPE_STRUCT = 0x20,
} TypeType;

typedef struct Members {
    uint32 memberCount;
    Type* members;
} Members;

typedef struct Type {
    TypeType type;

    // all
    AccessModifier access;
    LengthSpefifier length;

    // integer
    SignSpecifier sign;

    // struct union enum
    AvString name;

    // struct union
    Members members;

    // pointer
    Type* pointer;
} Type;

typedef struct TypeDefMapping {
    AvString typeName;
    Type value;
} TypeDefMapping;

#define matchToken(tokens, tokenCount, index, ...) matchToken_(tokens, tokenCount, index, __VA_ARGS__, 0)
bool32 matchToken_(Token* tokens, uint32 tokenCount, uint32 index, ...) {
    va_list types;
    va_start(types, index);
    TokenValueType type;
    while (type = va_arg(types, TokenValueType)) {
        if (type == tokenGetType(tokens[index])) {
            return true;
        }
    }
    va_end(types);
    return false;
}

void logError(const char* message) {
    avAssert(0, message);
    //TODO: better logging
    //WARNING: no longer exits when NDEBUG is specified
}

struct AccessExpression const* const a;

typedef struct TypeParsingData {
    Token* tokens;
    uint32 tokenCount;
    uint32 index;
    AV_DS(AvDynamicArray, TypeDefMapping) typedefs;
    AV_DS(AvDynamicArray, Type) types;
}TypeParsingData;

uint32 parseType(TypeParsingData data) {
    uint32 index = data.index;

    Type type = {
        .type = TYPE_INTEGER,
        .length = LENGTH_4b,
        .access = ACCESS_NORMAL,
        .sign = SPECIFIER_SIGNED,
        .pointer = nullptr,
        .members = AV_EMPTY,
        .name = AV_EMPTY
    };

    while (data.index < data.tokenCount) {
        switch (tokenGetType(data.tokens[index++])) {
        case TOKEN_VALUE_TYPE_CONST:
            if ((type.access & ACCESS_CONST) != 0) {
                type.access |= ACCESS_CONST;
                break;
            }
            logError("double const specified");
            break;
        case TOKEN_VALUE_TYPE_VOLATILE:
            if ((type.access & ACCESS_VOLATILE) != 0) {
                type.access |= ACCESS_VOLATILE;
                break;
            }
            logError("double volatile specified");
            break;
        case TOKEN_VALUE_TYPE_RESTRICT:
            if ((type.access & ACCESS_RESTRICT) != 0) {
                type.access |= ACCESS_RESTRICT;
                break;
            }
            logError("double restric specified");
            break;
        case TOKEN_VALUE_TYPE_SHORT:
            if ((type.length == LENGTH_4b)) {
                type.length = LENGTH_2b;
                break;
            }
            logError("double short specified");
            break;
        default: 
            logError("invalid token in type");
            break;
        }
    }

}
