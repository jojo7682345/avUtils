#include "avCLanguage.h"

#define KEYWORD_LIST \
    KEYWORD(AUTO, "auto")\
    KEYWORD(BREAK, "break")\
    KEYWORD(CASE, "case")\
    KEYWORD(CHAR, "char")\
    KEYWORD(CONST, "const")\
    KEYWORD(CONTINUE, "continue")\
    KEYWORD(DEFAULT, "default")\
    KEYWORD(DO, "do")\
    KEYWORD(DOUBLE, "double")\
    KEYWORD(ELSE, "else")\
    KEYWORD(ENUM, "enum")\
    KEYWORD(EXTERN, "extern")\
    KEYWORD(FLOAT, "float")\
    KEYWORD(FOR, "for")\
    KEYWORD(GOTO, "goto")\
    KEYWORD(IF, "if")\
    KEYWORD(INLINE, "inline")\
    KEYWORD(INT, "int")\
    KEYWORD(LONG, "long")\
    KEYWORD(REGISTER, "register")\
    KEYWORD(RESTRICT, "restrict")\
    KEYWORD(RETURN, "return")\
    KEYWORD(SHORT, "short")\
    KEYWORD(SIGNED, "signed")\
    KEYWORD(SIZEOF, "sizeof")\
    KEYWORD(STATIC, "static")\
    KEYWORD(STRUCT, "struct")\
    KEYWORD(SWITCH, "switch")\
    KEYWORD(TYPEDEF, "typedef")\
    KEYWORD(UNION, "union")\
    KEYWORD(UNSIGNED, "unsigned")\
    KEYWORD(VOID, "void")\
    KEYWORD(VOLATILE, "volatile")\
    KEYWORD(WHILE, "while")\

#define PUNCTUATOR_LIST \
    PUNCTUATOR(SQUARE_BRACKET_OPEN,"[")\
    PUNCTUATOR(SQUARE_BRACKET_CLOSE,"]")\
    PUNCTUATOR(BRACKET_OPEN,"(")\
    PUNCTUATOR(BRACKET_CLOSE,")")\
    PUNCTUATOR(CURLY_BRACKET_OPEN,"{")\
    PUNCTUATOR(CURLY_BRACKET_CLOSE,"}")\
    PUNCTUATOR(DOT,".")\
    PUNCTUATOR(ACCESS,"->")\
    PUNCTUATOR(INCREMENT,"++")\
    PUNCTUATOR(DECREMENT,"--")\
    PUNCTUATOR(AND,"&")\
    PUNCTUATOR(STAR,"*")\
    PUNCTUATOR(PLUS,"+")\
    PUNCTUATOR(MINUS,"-")\
    PUNCTUATOR(EXCLAMATION,"!")\
    PUNCTUATOR(SLASH,"/")\
    PUNCTUATOR(MODULO,"%")\
    PUNCTUATOR(LSHIFT,"<<")\
    PUNCTUATOR(RSHIFT,">>")\
    PUNCTUATOR(LESS_THAN,"<")\
    PUNCTUATOR(GREATER_THAN,">")\
    PUNCTUATOR(LESS_THAN_OR_EQUAL,"<=")\
    PUNCTUATOR(GREATHER_THAN_OR_EQUAL,">=")\
    PUNCTUATOR(EQUALS,"==")\
    PUNCTUATOR(NOT_EQUALS,"!=")\
    PUNCTUATOR(BITWISE_XOR,"^")\
    PUNCTUATOR(BITWISE_OR,"|")\
    PUNCTUATOR(LOGICAL_AND,"&&")\
    PUNCTUATOR(LOGICAL_OR,"||")\
    PUNCTUATOR(QUESTION_MARK,"?")\
    PUNCTUATOR(COLON,":")\
    PUNCTUATOR(SEMICOLON,";")\
    PUNCTUATOR(DOTDOTDOT,"...")\
    PUNCTUATOR(ASSIGN,"=")\
    PUNCTUATOR(MUL_ASSIGN,"*=")\
    PUNCTUATOR(DIV_ASSIGN,"/=")\
    PUNCTUATOR(MOD_ASSIGN,"%=")\
    PUNCTUATOR(ADD_ASSIGN,"+=")\
    PUNCTUATOR(SUB_ASSIGN,"-=")\
    PUNCTUATOR(LSH_ASSIGN,"<<==")\
    PUNCTUATOR(RSH_ASSIGN,">>==")\
    PUNCTUATOR(AND_ASSIGN,"&=")\
    PUNCTUATOR(XOR_ASSIGN,"^=")\
    PUNCTUATOR(OR_ASSIGN,"|=")\
    PUNCTUATOR(COMMA,",")\
    PUNCTUATOR(HASH,"#")\
    PUNCTUATOR(HASH_HASH,"##")\

#define INTEGER_SUFFIX_LIST \
    INTEGER_SUFFIX(UNSIGNED, "u")\
    INTEGER_SUFFIX(LONG, "l")\
    INTEGER_SUFFIX(LONG_LONG, "ll")\

typedef struct TokenList_T {
    uint64 tokenCount;
    struct Token tokens[];
}*TokenList;

TokenList tokenizeSource(AvString source);