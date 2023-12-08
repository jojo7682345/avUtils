#define AV_DYNAMIC_ARRAY_EXPOSE_MEMORY_LAYOUT
#include "avInterpreter.h"
#include "avTokenizer.h"
#include <AvUtils/avMemory.h>
#include <AvUtils/avLogging.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

typedef enum {
    EXPRESSION_TYPE_LITERAL,
    EXPRESSION_TYPE_UNARY,
    EXPRESSION_TYPE_BINARY,
    EXPRESSION_TYPE_GROUPING,
} ExpressionType;

typedef struct Expression_T* Expression;

typedef struct LiteralExpression {
    Token token;
} LiteralExpression;

typedef struct UnaryExpression {
    Expression expression;
    OperatorType operator;
} UnaryExpression;

typedef struct BinaryExpression {
    Expression left;
    OperatorType operator;
    Expression right;
}BinaryExpression;

typedef struct GroupingExpression {
    Expression content;
}GroupingExpression;

typedef struct Expression_T {
    ExpressionType type;
    union {
        LiteralExpression literal;
        UnaryExpression unary;
        BinaryExpression binary;
        GroupingExpression grouping;
    };
}Expression_T;

static Expression parseExpression(Token* tokens, uint32 tokenCount, uint32* index);

static Token previousToken(Token* tokens, uint32 tokenCount, uint32* index) {
    return tokens[(*index) - 1];
}

static Token peekToken(Token* tokens, uint32 tokenCount, uint32* index) {
    return tokens[*index];
}

static Token advanceToken(Token* tokens, uint32 tokenCount, uint32* index) {
    if ((*index) < tokenCount) {
        (*index)++;
    }
    return previousToken(tokens, tokenCount, index);
}

static bool32 checkToken(Token* tokens, uint32 tokenCount, uint32* index, TokenValueType type) {
    if ((*index) >= tokenCount) {
        return false;
    }
    return tokenGetType(peekToken(tokens, tokenCount, index)) == type;
}

static void throwError(Token token, const char* message) {
    if (token.type == TOKEN_NONE) {
        printf("Token ");
        avStringPrint(token.str);
        printf(" unexpected at end!\n%s\n", message);
        exit(-1);
    } else {
        printf("Token ");
        avStringPrint(token.str);
        printf(" unexpected!\n%s\n", message);
        exit(-1);
    }
}

static void synchroniseStatements(Token* tokens, uint32 tokenCount, uint32* index) {
    advanceToken(tokens, tokenCount, index);

    while ((*index) < tokenCount) {
        if (tokenGetType(previousToken(tokens, tokenCount, index)) == TOKEN_VALUE_TYPE_SEMICOLON){
            return;
        }
        switch (tokenGetType(peekToken(tokens, tokenCount, index))) {
        case TOKEN_VALUE_TYPE_IDENTIFIER:
        case TOKEN_VALUE_TYPE_FOR:
        case TOKEN_VALUE_TYPE_WHILE:
        case TOKEN_VALUE_TYPE_IF:
        case TOKEN_VALUE_TYPE_RETURN:
            return;
        default:
            advanceToken(tokens, tokenCount, index);
            break;
        }
    }
}

static Token consumeToken(Token* tokens, uint32 tokenCount, uint32* index, TokenValueType type, const char* msg) {
    if (checkToken(tokens, tokenCount, index, type)) return advanceToken(tokens, tokenCount, index);

    throwError(peekToken(tokens, tokenCount, index), msg);
}

#define matchToken(tokens, tokenCount, index, ...) matchToken_(tokens, tokenCount, index, __VA_ARGS__, TOKEN_VALUE_TYPE_NONE)
static bool32 matchToken_(Token* tokens, uint32 tokenCount, uint32* index, ...) {
    va_list args;
    va_start(args, index);
    TokenValueType arg;
    while ((arg = va_arg(args, TokenValueType)) != TOKEN_VALUE_TYPE_NONE) {
        if (checkToken(tokens, tokenCount, index, arg)) {
            advanceToken(tokens, tokenCount, index);
            return true;
        }
    }
    return false;
}

static Expression createExpression(Expression_T expr) {
    Expression expression = avCallocate(1, sizeof(Expression_T), "allocating expression");
    memcpy(expression, &expr, sizeof(Expression_T));
    return expression;
}

static Expression parsePrimary(Token* tokens, uint32 tokenCount, uint32* index) {
    if (matchToken(tokens, tokenCount, index,
        TOKEN_VALUE_TYPE_NUMBER_LITERAL,
        TOKEN_VALUE_TYPE_STRING_LITERAL,
        TOKEN_VALUE_TYPE_IDENTIFIER
    )) {
        return createExpression((Expression_T) {
            .type = EXPRESSION_TYPE_LITERAL,
                .literal.token = previousToken(tokens, tokenCount, index)
        });
    }
    if (matchToken(tokens, tokenCount, index,
        TOKEN_VALUE_TYPE_BRACKET_OPEN
    )) {
        Expression expr = parseExpression(tokens, tokenCount, index);
        consumeToken(tokens, tokenCount, index, TOKEN_VALUE_TYPE_BRACKET_CLOSE,
            "Expect ')' after expression."
        );
        return createExpression((Expression_T) {
            .type = EXPRESSION_TYPE_GROUPING,
                .grouping.content = expr
        });
    }
    throwError(peekToken(tokens, tokenCount, index), "Expected expression.");
}

static Expression parseUnary(Token* tokens, uint32 tokenCount, uint32* index) {
    if (matchToken(tokens, tokenCount, index,
        TOKEN_VALUE_TYPE_SUBTRACT,
        TOKEN_VALUE_TYPE_NOT
    )) {
        Token operator = previousToken(tokens, tokenCount, index);
        Expression right = parseUnary(tokens, tokenCount, index);
        return createExpression((Expression_T) {
            .type = EXPRESSION_TYPE_UNARY,
                .unary.expression = right,
                .unary.operator = tokenGetType(operator),

        });
    }
    return parsePrimary(tokens, tokenCount, index);
}

static Expression parseFactor(Token* tokens, uint32 tokenCount, uint32* index) {
    Expression expr = parseUnary(tokens, tokenCount, index);

    while (matchToken(tokens, tokenCount, index,
        TOKEN_VALUE_TYPE_MULTIPLY,
        TOKEN_VALUE_TYPE_DIVIDE
    )) {
        Token operator = previousToken(tokens, tokenCount, index);
        Expression right = parseUnary(tokens, tokenCount, index);
        expr = createExpression((Expression_T) {
            .type = EXPRESSION_TYPE_BINARY,
                .binary.left = expr,
                .binary.operator = tokenGetType(operator),
                .binary.right = right
        });
    }

    return expr;
}

static Expression parseTerm(Token* tokens, uint32 tokenCount, uint32* index) {
    Expression expr = parseFactor(tokens, tokenCount, index);

    while (matchToken(tokens, tokenCount, index,
        TOKEN_VALUE_TYPE_SUBTRACT,
        TOKEN_VALUE_TYPE_ADD
    )) {
        Token operator = previousToken(tokens, tokenCount, index);
        Expression right = parseFactor(tokens, tokenCount, index);
        expr = createExpression((Expression_T) {
            .type = EXPRESSION_TYPE_BINARY,
                .binary.left = expr,
                .binary.operator = tokenGetType(operator),
                .binary.right = right
        });
    }

    return expr;
}

static Expression parseComparison(Token* tokens, uint32 tokenCount, uint32* index) {
    Expression expr = parseTerm(tokens, tokenCount, index);

    while (matchToken(tokens, tokenCount, index,
        TOKEN_VALUE_TYPE_GREATER_THAN,
        TOKEN_VALUE_TYPE_LESS_THAN,
        TOKEN_VALUE_TYPE_GREATER_THAN_OR_EQUAL,
        TOKEN_VALUE_TYPE_LESS_THAN_OR_EQUAL
    )) {
        Token operator = previousToken(tokens, tokenCount, index);
        Expression right = parseTerm(tokens, tokenCount, index);
        expr = createExpression((Expression_T) {
            .type = EXPRESSION_TYPE_BINARY,
                .binary.left = expr,
                .binary.operator = tokenGetType(operator),
                .binary.right = right
        });
    }
    return expr;
}

static Expression parseEquality(Token* tokens, uint32 tokenCount, uint32* index) {
    Expression expr = parseComparison(tokens, tokenCount, index);

    while (matchToken(tokens, tokenCount, index, TOKEN_VALUE_TYPE_NOT_EQUALS, TOKEN_VALUE_TYPE_EQUALS)) {
        Token operator = previousToken(tokens, tokenCount, index);

        Expression right = parseComparison(tokens, tokenCount, index);
        expr = createExpression((Expression_T) {
            .type = EXPRESSION_TYPE_BINARY,
                .binary.left = expr,
                .binary.operator = tokenGetType(operator),
                .binary.right = right
        });
    }

    return expr;
}

static Expression parseExpression(Token* tokens, uint32 tokenCount, uint32* index) {
    return parseEquality(tokens, tokenCount, index);
}

static Expression parse(Token* tokens, uint32 tokenCount, uint32* index){
    return parseExpression(tokens, tokenCount, index);
}

AvParseResult avBuildSyntaxTree(AvCompileData data) {
    Token nullToken = {
        .str = AV_CSTR(""),
        .type = TOKEN_NONE
    };
    avDynamicArrayAdd(&nullToken, data->tokens);
    avDynamicArrayMakeContiguous(data->tokens);
    Token* tokens = avDynamicArrayGetPageDataPtr(0, data->tokens);
    uint32 tokenCount = avDynamicArrayGetSize(data->tokens);
    uint32 index = 0;
    Expression expr = parse(tokens, tokenCount, &index);

}