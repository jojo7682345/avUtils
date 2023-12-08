#ifndef __AV_TOKENIZER__
#define __AV_TOKENIZER__
#include <AvUtils/avDefinitions.h>
C_SYMBOLS_START
#include <AvUtils/avTypes.h>
#include <AvUtils/avString.h>
#include <AvUtils/avBuilder.h>
#include "avInterpreter.h"

#define TOKEN_LIST \
    TOKEN(PRE_PROCESSOR, MATCH, PRE_PROCESSOR_LIST)\
    TOKEN(KEYWORD, MATCH, KEYWORD_LIST)\
    TOKEN(SPECIAL, MATCH, SPECIAL_LIST)\
    TOKEN(OPERATOR, MATCH, OPERATOR_LIST)\
    TOKEN(NEWLINE, MATCH, "\\\n")\
    TOKEN(NUMBER, NUMBER, 0)\
    TOKEN(STRING, STRING, 0)\
    TOKEN(IDENTIFIER, RANGE, "a-z","A-Z", "_-_", "0-9")\

#define PRE_PROCESSOR_LIST \
    PRE_PROCESSOR(INCLUDE, "include")\
    PRE_PROCESSOR(DEFINE, "define")\
    PRE_PROCESSOR(UNDEF, "undef")\

#define KEYWORD_LIST \
    KEYWORD(IF, "if")\
    KEYWORD(CONST ,"const")\
    KEYWORD(STRUCT,"struct")\
    KEYWORD(ELSE,"else")\
    KEYWORD(ENUM,"enum")\
    KEYWORD(TYPEDEF,"typedef")\
    KEYWORD(WHILE,"while")\
    KEYWORD(FOR,"for")\
    KEYWORD(SWITCH,"switch")\
    KEYWORD(CASE,"case")\
    KEYWORD(DEFAULT,"default")\
    KEYWORD(RETURN, "return")\

#define SPECIAL_LIST \
    SPECIAL(SQUARE_BRACKET_OPEN,"[")\
    SPECIAL(SQUARE_BRACKET_CLOSE,"]")\
    SPECIAL(CURLY_BRACKET_OPEN,"{")\
    SPECIAL(CURLY_BRACKET_CLOSE,"}")\
    SPECIAL(BRACKET_OPEN,"(")\
    SPECIAL(BRACKET_CLOSE,")")\
    SPECIAL(COMMA,",")\
    SPECIAL(SEMICOLON,";")\

#define OPERATOR_LIST \
    OPERATOR(DIVIDE, "/")\
    OPERATOR(SUBTRACT, "-")\
    OPERATOR(ACCESS, "->")\
    OPERATOR(DOT,".")\
    OPERATOR(QUESTION_MARK,"?")\
    OPERATOR(COLON,":")\
    OPERATOR(MODULO, "%")\
    OPERATOR(ADD, "+")\
    OPERATOR(EQUALS, "==")\
    OPERATOR(NOT_EQUALS, "!=")\
    OPERATOR(LESS_THAN, "<")\
    OPERATOR(GREATER_THAN, ">")\
    OPERATOR(LESS_THAN_OR_EQUAL, "<=")\
    OPERATOR(GREATER_THAN_OR_EQUAL, ">=")\
    OPERATOR(AND, "&&")\
    OPERATOR(OR, "||")\
    OPERATOR(NOT, "!")\
    OPERATOR(INC, "++")\
    OPERATOR(DEC, "--")\
    OPERATOR(LEFT_SHIFT, "<<")\
    OPERATOR(RIGHT_SHIFT, ">>")\
    OPERATOR(BITWISE_NOT, "~")\
    OPERATOR(BITWISE_XOR, "^")\
    OPERATOR(BITWISE_AND, "&")\
    OPERATOR(BITWISE_OR, "|")\
    OPERATOR(MULTIPLY, "*")\
    OPERATOR(ASSIGN, "=")\
    OPERATOR(ADD_ASSIGN, "+=")\
    OPERATOR(SUBTRACT_ASSIGN, "-=")\
    OPERATOR(MULTIPLY_ASSIGN, "*=")\
    OPERATOR(DIVIDE_ASSIGN, "/=")\
    OPERATOR(MODULO_ASSIGN, "%=")\
    OPERATOR(LEFT_SHIFT_ASSIGN, "<<=")\
    OPERATOR(RIGHT_SHIFT_ASSIGN, ">>=")\
    OPERATOR(BITWISE_AND_ASSIGN, "&=")\
    OPERATOR(BITWISE_XOR_ASSIGN, "^=")\
    OPERATOR(BITWISE_OR_ASSIGN, "|=")\
    OPERATOR(BITWISE_NOT_ASSIGN, "~=")\

typedef enum TokenOpType {
    MATCH,
    RANGE,
    NUMBER,
    STRING,
}TokenOpType;

enum { TOKEN_TYPE_COUNTER_BASE = __COUNTER__ };
#define TOKEN_TYPE_LOCAL_COUNTER (__COUNTER__ - TOKEN_TYPE_COUNTER_BASE - 1)
#define TOKEN(token, OP, ...) TOKEN_##token = (1 << TOKEN_TYPE_LOCAL_COUNTER),
typedef enum TokenType {
    TOKEN_NONE = 0,
    TOKEN_LIST
} TokenType;
#undef TOKEN

enum { TOKEN_VALUE_TYPE_COUNTER_BASE = __COUNTER__ };
#define TOKEN_VALUE_TYPE_LOCAL_COUNTER (__COUNTER__ - TOKEN_VALUE_TYPE_COUNTER_BASE - 1)

#define PRE_PROCESSOR_VALUE_TYPE 0x1
#define KEYWORD_VALUE_TYPE 0x2
#define SPECIAL_VALUE_TYPE 0x4
#define OPERATOR_VALUE_TYPE 0x8
#define PRE_PROCESSOR(name, str)  TOKEN_VALUE_TYPE_##name = (TOKEN_VALUE_TYPE_LOCAL_COUNTER << 4) | PRE_PROCESSOR_VALUE_TYPE,
#define KEYWORD(name, str)  TOKEN_VALUE_TYPE_##name = (TOKEN_VALUE_TYPE_LOCAL_COUNTER << 4) | KEYWORD_VALUE_TYPE,
#define SPECIAL(name, str)  TOKEN_VALUE_TYPE_##name = (TOKEN_VALUE_TYPE_LOCAL_COUNTER << 4) | SPECIAL_VALUE_TYPE,
#define OPERATOR(name, str)  TOKEN_VALUE_TYPE_##name = (TOKEN_VALUE_TYPE_LOCAL_COUNTER << 4) | OPERATOR_VALUE_TYPE, 
typedef enum TokenValueType {
    TOKEN_VALUE_TYPE_NONE = 0,
    TOKEN_VALUE_TYPE_IDENTIFIER = 1 << 4,
    TOKEN_VALUE_TYPE_NUMBER_LITERAL = 2 << 4,
    TOKEN_VALUE_TYPE_STRING_LITERAL = 3 << 4,
    TOKEN_VALUE__PRE_PROCESSOR = (TOKEN_VALUE_TYPE_LOCAL_COUNTER << 4) | PRE_PROCESSOR_VALUE_TYPE,
    PRE_PROCESSOR_LIST
    TOKEN_VALUE__KEYWORD = (TOKEN_VALUE_TYPE_LOCAL_COUNTER << 4) | KEYWORD_VALUE_TYPE,
    KEYWORD_LIST
    TOKEN_VALUE__SPECIAL = (TOKEN_VALUE_TYPE_LOCAL_COUNTER << 4) | SPECIAL_VALUE_TYPE,
    SPECIAL_LIST
    TOKEN_VALUE__OPERATOR = (TOKEN_VALUE_TYPE_LOCAL_COUNTER << 4) | OPERATOR_VALUE_TYPE,
    OPERATOR_LIST
} TokenValueType;
#undef OPERATOR
#undef SPECIAL
#undef KEYWORD
#undef PRE_PROCESSOR


typedef struct TokenFormat {
    TokenType type;
    TokenOpType op;
    uint32 valueCount;
    const char** values;
} TokenFormat;

typedef struct Token {
    TokenType type;
    AvString str;
} Token;

#define PRE_PROCESSOR(name, str)  PRE_PROCESSOR_##name,
#define KEYWORD(name, str)  KEYWORD_##name,
#define SPECIAL(name, str)  SPECIAL_##name,
#define OPERATOR(name, str)  OPERATOR_##name,
typedef enum OperatorType {
    OPERATOR_LIST
}OperatorType;

typedef enum PreProcessorType {
    PRE_PROCESSOR_LIST
}PreProcessorType;

typedef enum KeywordType {
    KEYWORD_LIST
}KeywordType;

typedef enum SpecialType {
    SPECIAL_LIST
}SpecialType;
#undef OPERATOR
#undef SPECIAL
#undef KEYWORD
#undef PRE_PROCESSOR

extern const char* tokenTypeStrs[];
extern const TokenFormat tokenFormats[];
extern const uint32 tokenFormatCount;

extern AvString operatorStrs[];

AvParseResult avTokenize(AvString file, AvCompileData data);

bool32 tokenIsType(Token token, TokenValueType type);

TokenValueType tokenGetType(Token token);

C_SYMBOLS_END
#endif//__AV_TOKENIZER__


