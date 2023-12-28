#define AV_DYNAMIC_ARRAY_EXPOSE_MEMORY_LAYOUT
#include "avInterpreter.h"
#include "avTokenizer.h"
#include <AvUtils/avMemory.h>
#include <AvUtils/avLogging.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>


/*
[x] program         -> statement* EOF ;

[ ] statement       -> defStatement | exprStatement | block | typedefStatement;
[x] exprStatement   -> expression? ";" ;
[ ] returnStatement -> return expression ";" ;
[x] defStatement    -> storageClass? type IDENTIFIER ( "(" arguments? ")" block | ";" ) | ( ( "=" expression )? ";" ) ;
[x] typedefStatement-> "typedef" type IDENTIFIER ";" ;
[x] block           -> "{" statement* "}" ;
[x] arguments       -> argument ("," argument)* ;
[x] argument        -> type IDENTIFIER ;
[x] parameters      -> expression ( "," expression )* ;
[x] type            -> accessModifier ( structType | enumType | unionType | IDENTIFIER ) pointer? ;
[x] structType      -> "struct" ( ( IDENTIFIER? "{" defStatement* "}" ) | IDENTIFIER ) ;
[x] pointer         -> "*" accessModifier? ;
[x] enumType        -> "enum" (( IDENTIFIER? "{" enumElements "}") | IDENTIFIER ) ;
[x] enumElements    -> enumElement (, enumElement)* ;
[x] enumElement     -> IDENTIFIER ( "=" expression )? ;
[x] unionType       -> "union" (( IDENTIFIER? "{" defStatement* "}" ) | IDENTIFIER ) ;
[x] accessModifier  -> "const"? "volatile"? ;
[x] storageClass    -> "extern" | "static" ;

[x] expression      -> equality ;
[x] equality        -> comparison ( ( "!=" | "==" ) comparison )* ;
[x] comparison      -> term ( ( ">" | ">=" | "<" | "<=" ) term )* ;
[x] term            -> factor ( ( "-" | "+" ) factor )* ;
[x] factor          -> unary ( ( "/" | "*" ) unary )* ;
[x] unary           -> ( "!" | "-" ) unary | call ;
[x] call            -> primary | ( call "(" parameters? ")" ) ;
[x] primary         -> NUMBER | STRING | IDENTIFIER | "(" expression ")" ;
*/

typedef enum {
    EXPRESSION_TYPE_LITERAL,
    EXPRESSION_TYPE_UNARY,
    EXPRESSION_TYPE_BINARY,
    EXPRESSION_TYPE_GROUPING,
    EXPRESSION_TYPE_CALL,
    EXPRESSION_TYPE_ACCESS,
} ExpressionType;

typedef struct Expression_T* Expression;

typedef struct LiteralExpression {
    Token token;
} LiteralExpression;

typedef struct ParameterList {
    Expression param;
    struct ParameterList* next;
} ParameterList;

typedef struct CallExpression {
    Expression function;
    ParameterList* parameters;
}CallExpression;

typedef struct AccessExpression {
    Expression base;
    TokenValueType type;
    Expression offset;
}AccessExpression;

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
        CallExpression call;
        AccessExpression access;
    };
}Expression_T;

typedef enum StatementType {
    STATEMENT_TYPE_EXPRESSION,
    STATEMENT_TYPE_DEFINITION,
    STATEMENT_TYPE_TYPEDEF,
    STATEMENT_TYPE_RETURN,
}StatementType;

typedef struct ExpressionStatement {
    Expression expression;
}ExpressionStatement;

typedef struct EnumElement {
    Token identifier;
    Expression value;
} EnumElement;

typedef struct EnumElements {
    EnumElement element;
    struct EnumElements* next;
}EnumElements;

typedef struct Statement_T* Statement;

typedef struct DefinitionStatementList {
    struct DefinitionStatementList* next;
    Statement member;
}DefinitionStatementList;

typedef struct Enum {
    Token identifier;
    EnumElements* elements;
} Enum;

typedef enum PointerType {
    POINTER_TYPE_NONE = 0,
    POINTER_TYPE_NORMAL = 1,
    POINTER_TYPE_CONST = 2,
} PointerType;

typedef enum StorageClass {
    STORAGE_CLASS_NONE = 0,
    STORAGE_CLASS_EXTERN = 1,
    STORAGE_CLASS_STATIC = 2,
} StorageClass;

typedef enum AccessModifier {
    ACCESS_MODIFIER_NONE = 0,
    ACCESS_MODIFIER_VOLATILE = 1,
    ACCESS_MODIFIER_CONST = 2,
} AccessModifier;

typedef enum TypeType {
    TYPE_INVALID = 0,
    TYPE_STRUCT = 1,
    TYPE_ENUM = 2,
    TYPE_UNION = 3,
    TYPE_IDENTIFIER = 4,
}TypeType;

typedef struct Struct {
    Token identifier;
    DefinitionStatementList* members;
} Struct;

typedef struct Union {
    Token identifier;
    DefinitionStatementList* members;
} Union;

typedef struct Type {
    AccessModifier accessModifier;
    TypeType type;
    union {
        Enum enumType;
        Struct structType;
        Union unionType;
        Token identifier;
    };
    PointerType pointer;
}Type;

typedef struct ASTdata {
    AV_DS(AvDynamicArray, typedefs) typedefs;
    Token* tokens;
    uint32 tokenCount;
    uint32* index;
} ASTdata;

typedef enum DefinitionType {
    DEFINITION_TYPE_VARIABLE,
    DEFINITION_TYPE_FUNCTION,
}DefinitionType;

typedef struct Argument {
    Type type;
    Token identifier;
} Argument;

typedef struct ArgumentList {
    Argument arg;
    struct ArgumentList* next;
} ArgumentList;

typedef struct Block {
    struct StatementList* statements;
} Block;

typedef struct DefinitionStatement {
    StorageClass storageClass;
    Type type;
    Token identifier;
    DefinitionType definitionType;
    union {
        ExpressionStatement variableInitializer;
        struct {
            ArgumentList* arguments;
            bool32 containsFunctionBody;
            Block functionBody;
        };
    };
} DefinitionStatement;

typedef struct TypedefStatement {
    Type type;
    Token identifier;
}TypedefStatement;



typedef struct FunctionDeclarationStatement {
    Type returnType;
    Token identifier;
    ArgumentList* arguments;
    Block block;
}FunctionDeclarationStatement;

typedef struct ReturnStatement {
    Expression expression;
} ReturnStatement;

typedef struct Statement_T {
    StatementType type;
    union {
        ExpressionStatement expression;
        DefinitionStatement definition;
        TypedefStatement typedefStmnt;
        ReturnStatement returnStatement;
    };
}Statement_T;

typedef struct StatementList {
    Statement statement;
    struct StatementList* next;
}StatementList;

static Expression parseExpression(ASTdata data);

static Token previousToken(ASTdata data) {
    return data.tokens[(*data.index) - 1];
}

static Token peekToken(ASTdata data) {
    if ((*data.index) >= data.tokenCount) {
        return (Token) { 0 };
    }
    return data.tokens[*data.index];
}

static Token advanceToken(ASTdata data) {
    if ((*data.index) < data.tokenCount) {
        (*data.index)++;
    }
    return previousToken(data);
}

static bool32 checkToken(ASTdata data, TokenValueType type) {
    if ((*data.index) >= data.tokenCount) {
        return false;
    }
    TokenValueType tokenType = tokenGetType(peekToken(data));
    return tokenType == type;
}

bool32 isType(ASTdata data, Token token) {
    for (uint32 index = 0; index < avDynamicArrayGetSize(data.typedefs); index++) {
        AvString element = { 0 };
        avDynamicArrayRead(&element, index, data.typedefs);
        if (avStringEquals(element, token.str)) {
            return true;
        }
    }
    return false;
}

void addType(ASTdata data, Token token) {
    if (!isType(data, token)) {
        avDynamicArrayAdd(&token.str, data.typedefs);
    }
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

// static void synchroniseStatements(ASTdata data) {
//     advanceToken(data);

//     while ((*data.index) < data.tokenCount) {
//         if (tokenGetType(previousToken(data)) == TOKEN_VALUE_TYPE_SEMICOLON) {
//             return;
//         }
//         switch (tokenGetType(peekToken(data))) {
//         case TOKEN_VALUE_TYPE_IDENTIFIER:
//         case TOKEN_VALUE_TYPE_FOR:
//         case TOKEN_VALUE_TYPE_WHILE:
//         case TOKEN_VALUE_TYPE_IF:
//         case TOKEN_VALUE_TYPE_RETURN:
//             return;
//         default:
//             advanceToken(data);
//             break;
//         }
//     }
// }

static Token consumeToken(ASTdata data, TokenValueType type, const char* msg) {
    if (checkToken(data, type)) return advanceToken(data);

    throwError(peekToken(data), msg);
    return (Token){0};
}

#define matchToken(data, ...) matchToken_(data, __VA_ARGS__, TOKEN_VALUE_TYPE_NONE)
static bool32 matchToken_(ASTdata data, ...) {
    va_list args;
    va_start(args, data);
    TokenValueType arg;
    while ((arg = va_arg(args, TokenValueType)) != TOKEN_VALUE_TYPE_NONE) {
        if (checkToken(data, arg)) {
            advanceToken(data);
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

static Statement createStatement(Statement_T statement) {
    Statement stmnt = avCallocate(1, sizeof(Statement_T), "allocating statement");
    memcpy(stmnt, &statement, sizeof(Statement_T));
    return stmnt;
}

static Expression parsePrimary(ASTdata data) {
    if (matchToken(data,
        TOKEN_VALUE_TYPE_NUMBER_LITERAL,
        TOKEN_VALUE_TYPE_STRING_LITERAL
    )) {
        return createExpression((Expression_T) {
            .type = EXPRESSION_TYPE_LITERAL,
                .literal.token = previousToken(data)
        });
    }
    if (matchToken(data, TOKEN_VALUE_TYPE_IDENTIFIER)) {
        return createExpression((Expression_T) {
            .type = EXPRESSION_TYPE_LITERAL,
                .literal.token = previousToken(data)
        });
    }
    if (matchToken(data,
        TOKEN_VALUE_TYPE_BRACKET_OPEN
    )) {
        Expression expr = parseExpression(data);
        consumeToken(data, TOKEN_VALUE_TYPE_BRACKET_CLOSE,
            "Expect ')' after expression."
        );
        return createExpression((Expression_T) {
            .type = EXPRESSION_TYPE_GROUPING,
                .grouping.content = expr
        });
    }
    throwError(peekToken(data), "Expected expression.");
    return nullptr;
}

static ParameterList* parseParameters(ASTdata data);

static Expression parseCall(ASTdata data) {
    Expression expr = parsePrimary(data);

    if (matchToken(data, TOKEN_VALUE_TYPE_BRACKET_OPEN)) {
        CallExpression call = {
            .function = expr
        };
        call.parameters = parseParameters(data);
        expr = createExpression((Expression_T) {
            .type = EXPRESSION_TYPE_CALL,
                .call = call
        });
        consumeToken(data, TOKEN_VALUE_TYPE_BRACKET_CLOSE, "expected ')'!");
    }

    return expr;
}

static Expression parseAccess(ASTdata data){
    Expression expr = parseCall(data);
    if(matchToken(data, 
        TOKEN_VALUE_TYPE_DOT,
        TOKEN_VALUE_TYPE_ACCESS
    )){
        Token operator = previousToken(data);
        Expression right = parseCall(data);
        expr = createExpression((Expression_T) {
            .type = EXPRESSION_TYPE_ACCESS,
                .access.base = expr,
                .access.type = tokenGetType(operator),
                .access.offset = right
        });  
    }
    if(matchToken(data, TOKEN_VALUE_TYPE_SQUARE_BRACKET_OPEN)){
        Expression offset = parseExpression(data);
        expr = createExpression((Expression_T) {
            .type = EXPRESSION_TYPE_ACCESS,
                .access.base = expr,
                .access.type = TOKEN_VALUE_TYPE_SQUARE_BRACKET_OPEN,
                .access.offset = offset
        });
        consumeToken(data, TOKEN_VALUE_TYPE_SQUARE_BRACKET_CLOSE, "expected ']'!");
    }
    return expr;
}

static Expression parseUnary(ASTdata data) {
    if (matchToken(data,
        TOKEN_VALUE_TYPE_SUBTRACT,
        TOKEN_VALUE_TYPE_ADD,
        TOKEN_VALUE_TYPE_NOT,
        TOKEN_VALUE_TYPE_INC,
        TOKEN_VALUE_TYPE_DEC,
        TOKEN_VALUE_TYPE_BITWISE_NOT,
        TOKEN_VALUE_TYPE_MULTIPLY,
        TOKEN_VALUE_TYPE_AND
    )) {
        Token operator = previousToken(data);
        Expression right = parseUnary(data);
        return createExpression((Expression_T) {
            .type = EXPRESSION_TYPE_UNARY,
                .unary.expression = right,
                .unary.operator = ((tokenGetType(operator) - TOKEN_VALUE__OPERATOR) >> 4) - 1,

        });
    }
    return parseAccess(data);
}

static Expression parseFactor(ASTdata data) {
    Expression expr = parseUnary(data);

    while (matchToken(data,
        TOKEN_VALUE_TYPE_MULTIPLY,
        TOKEN_VALUE_TYPE_DIVIDE,
        TOKEN_VALUE_TYPE_MODULO
    )) {
        Token operator = previousToken(data);
        Expression right = parseUnary(data);
        expr = createExpression((Expression_T) {
            .type = EXPRESSION_TYPE_BINARY,
                .binary.left = expr,
                .binary.operator = ((tokenGetType(operator) - TOKEN_VALUE__OPERATOR) >> 4) - 1,
                .binary.right = right
        });
    }

    return expr;
}

static Expression parseTerm(ASTdata data) {
    Expression expr = parseFactor(data);

    while (matchToken(data,
        TOKEN_VALUE_TYPE_SUBTRACT,
        TOKEN_VALUE_TYPE_ADD
    )) {
        Token operator = previousToken(data);
        Expression right = parseFactor(data);
        expr = createExpression((Expression_T) {
            .type = EXPRESSION_TYPE_BINARY,
                .binary.left = expr,
                .binary.operator = ((tokenGetType(operator) - TOKEN_VALUE__OPERATOR) >> 4) - 1,
                .binary.right = right
        });
    }

    return expr;
}

static Expression parseComparison(ASTdata data) {
    Expression expr = parseTerm(data);

    while (matchToken(data,
        TOKEN_VALUE_TYPE_GREATER_THAN,
        TOKEN_VALUE_TYPE_LESS_THAN,
        TOKEN_VALUE_TYPE_GREATER_THAN_OR_EQUAL,
        TOKEN_VALUE_TYPE_LESS_THAN_OR_EQUAL
    )) {
        Token operator = previousToken(data);
        Expression right = parseTerm(data);
        expr = createExpression((Expression_T) {
            .type = EXPRESSION_TYPE_BINARY,
                .binary.left = expr,
                .binary.operator = ((tokenGetType(operator) - TOKEN_VALUE__OPERATOR) >> 4) - 1,
                .binary.right = right
        });
    }
    return expr;
}

static Expression parseEquality(ASTdata data) {
    Expression expr = parseComparison(data);

    while (matchToken(data, TOKEN_VALUE_TYPE_NOT_EQUALS, TOKEN_VALUE_TYPE_EQUALS)) {
        Token operator = previousToken(data);

        Expression right = parseComparison(data);
        expr = createExpression((Expression_T) {
            .type = EXPRESSION_TYPE_BINARY,
                .binary.left = expr,
                .binary.operator = ((tokenGetType(operator) - TOKEN_VALUE__OPERATOR) >> 4) - 1,
                .binary.right = right
        });
    }

    return expr;
}

static Type parseType(ASTdata data);

static Statement parseTypedefStatement(ASTdata data) {
    Type type = parseType(data);
    Token identifier = consumeToken(data, TOKEN_VALUE_TYPE_IDENTIFIER, "Expected identifier!");
    consumeToken(data, TOKEN_VALUE_TYPE_SEMICOLON, "Expected ';'!");
    addType(data, identifier);
    return createStatement((Statement_T) {
        .type = STATEMENT_TYPE_TYPEDEF,
            .typedefStmnt.type = type,
            .typedefStmnt.identifier = identifier
    });
}

static Argument parseArgument(ASTdata data) {
    Type type = parseType(data);
    Token identifier = consumeToken(data, TOKEN_VALUE_TYPE_IDENTIFIER, "expected identifier!");

    return (Argument) {
        .type = type,
            .identifier = identifier
    };
}

static ArgumentList* parseArguments(ASTdata data) {
    ArgumentList* args = avCallocate(1, sizeof(ArgumentList), "allocating argument list");
    Argument arg = parseArgument(data);
    memcpy(&args->arg, &arg, sizeof(Argument));
    ArgumentList* next = args;
    while (matchToken(data, TOKEN_VALUE_TYPE_COMMA)) {
        next->next = avCallocate(1, sizeof(ArgumentList), "allocating argument list");
        next = next->next;
        Argument arg_ = parseArgument(data);
        memcpy(&next->arg, &arg_, sizeof(Argument));
    }
    return args;
}

static struct DefinitionStatementList* parseMembers(ASTdata data);

static Union parseUnion(ASTdata data) {
    if (matchToken(data, TOKEN_VALUE_TYPE_IDENTIFIER)) {
        Union unionType = {
            .identifier = previousToken(data)
        };
        if (matchToken(data, TOKEN_VALUE_TYPE_CURLY_BRACKET_OPEN)) {
            unionType.members = parseMembers(data);
            consumeToken(data, TOKEN_VALUE_TYPE_CURLY_BRACKET_CLOSE, "expected '}'!");
        }
        return unionType;
    } else {
        consumeToken(data, TOKEN_VALUE_TYPE_CURLY_BRACKET_OPEN, "expected '{'!");
        DefinitionStatementList* members = parseMembers(data);
        consumeToken(data, TOKEN_VALUE_TYPE_CURLY_BRACKET_CLOSE, "expected '}'!");
        return (Union) {
            .identifier = { 0 },
                .members = members
        };
    }
}

static Statement parseDefinitionStatement(ASTdata data);

static struct DefinitionStatementList* parseMembers(ASTdata data) {
    DefinitionStatementList* list = avCallocate(1, sizeof(DefinitionStatementList), "allocating definitionStatement");
    list->member = parseDefinitionStatement(data);
    DefinitionStatementList* next = list;
    while (!checkToken(data, TOKEN_VALUE_TYPE_CURLY_BRACKET_CLOSE)) {
        next->next = avCallocate(1, sizeof(DefinitionStatementList), "allocating definition statement");
        next = next->next;
        next->member = parseDefinitionStatement(data);
    };
    return list;
}

static Struct parseStruct(ASTdata data) {

    if (matchToken(data, TOKEN_VALUE_TYPE_IDENTIFIER)) {
        Struct structType = {
            .identifier = previousToken(data)
        };
        if (matchToken(data, TOKEN_VALUE_TYPE_CURLY_BRACKET_OPEN)) {
            structType.members = parseMembers(data);
            consumeToken(data, TOKEN_VALUE_TYPE_CURLY_BRACKET_CLOSE, "expected '}'!");
        }
        return structType;
    } else {
        consumeToken(data, TOKEN_VALUE_TYPE_CURLY_BRACKET_OPEN, "expected '{'!");
        DefinitionStatementList* members = parseMembers(data);
        consumeToken(data, TOKEN_VALUE_TYPE_CURLY_BRACKET_CLOSE, "expected '}'!");
        return (Struct) {
            .identifier = { 0 },
                .members = members
        };
    }
}

static EnumElements* parseEnumElements(ASTdata data);

static Enum parseEnum(ASTdata data) {

    if (matchToken(data, TOKEN_VALUE_TYPE_IDENTIFIER)) {
        Token identifier = previousToken(data);
        EnumElements* elements = nullptr;
        if (matchToken(data, TOKEN_VALUE_TYPE_CURLY_BRACKET_OPEN)) {
            elements = parseEnumElements(data);
            consumeToken(data, TOKEN_VALUE_TYPE_CURLY_BRACKET_CLOSE, "expected '}'!");
        }
        return (Enum) {
            .elements = elements,
                .identifier = identifier
        };
    } else {
        consumeToken(data, TOKEN_VALUE_TYPE_CURLY_BRACKET_OPEN, "expected '{'!");
        EnumElements* elements = parseEnumElements(data);
        consumeToken(data, TOKEN_VALUE_TYPE_CURLY_BRACKET_CLOSE, "expected '}'!");
        return (Enum) {
            .elements = elements,
                .identifier = { 0 }
        };
    }
}

//static AccessModifier parseAccessModifier(ASTdata data);
static PointerType parsePointerType(ASTdata data);

static Type parseType(ASTdata data) {
    //AccessModifier accessModifier = parseAccessModifier(data);
    Type type = { 0 };
    type.type = TYPE_INVALID;

    if (matchToken(data, TOKEN_VALUE_TYPE_STRUCT)) {
        type.type = TYPE_STRUCT;
        Struct structValue = parseStruct(data);
        memcpy(&type.structType, &structValue, sizeof(Struct));
    } else if (matchToken(data, TOKEN_VALUE_TYPE_ENUM)) {
        type.type = TYPE_ENUM;
        Enum enumValue = parseEnum(data);
        memcpy(&type.enumType, &enumValue, sizeof(Enum));
    } else if (matchToken(data, TOKEN_VALUE_TYPE_UNION)) {
        type.type = TYPE_UNION;
        Union unionValue = parseUnion(data);
        memcpy(&type.unionType, &unionValue, sizeof(Union));
    } else if (matchToken(data, TOKEN_VALUE_TYPE_IDENTIFIER)) {
        type.type = TYPE_IDENTIFIER;
        Token identifier = previousToken(data);
        if (!isType(data, identifier)) {
            throwError(identifier, "type is unknown");
        }
        memcpy(&type.identifier, &identifier, sizeof(Token));
    } else if (type.type == TYPE_INVALID) {
        throwError(peekToken(data), "unexpected token!");
        return type;
    }
    type.pointer = parsePointerType(data);
    return type;
}

static Statement parseStatement(ASTdata data);

static Block parseBlock(ASTdata data) {
    //consumeToken(data, TOKEN_VALUE_TYPE_CURLY_BRACKET_OPEN, "expected '{'!");

    StatementList* statements = avCallocate(1, sizeof(StatementList), "allocating statementList");
    StatementList* next = statements;
    while (*data.index < data.tokenCount && !checkToken(data, TOKEN_VALUE_TYPE_CURLY_BRACKET_CLOSE)) {
        next->next = avCallocate(1, sizeof(StatementList), "allocating statementList");
        next = next->next;
        next->statement = parseStatement(data);
    }

    //consumeToken(data, TOKEN_VALUE_TYPE_CURLY_BRACKET_CLOSE, "expected '{'!");
    return (Block) {
        .statements = statements
    };
}

static ParameterList* parseParameters(ASTdata data) {
    if (checkToken(data, TOKEN_VALUE_TYPE_BRACKET_CLOSE)) {
        return nullptr;
    }
    ParameterList* list = avCallocate(1, sizeof(ParameterList), "allocating parameter list");
    Expression expr = parseExpression(data);
    list->param = expr;

    ParameterList* next = list;
    while (matchToken(data, TOKEN_VALUE_TYPE_COMMA)) {
        next->next = avCallocate(1, sizeof(ParameterList), "allocating parameter list");
        next = next->next;
        next->param = parseExpression(data);
    }
    return list;
}

static PointerType parsePointerType(ASTdata data) {
    if (!matchToken(data, TOKEN_VALUE_TYPE_MULTIPLY)) {
        return POINTER_TYPE_NONE;
    }
    if (matchToken(data, TOKEN_VALUE_TYPE_CONST)) {
        return POINTER_TYPE_CONST;
    }
    return POINTER_TYPE_NORMAL;
}


static EnumElement parseEnumElement(ASTdata data);

static EnumElements* parseEnumElements(ASTdata data) {
    EnumElement element = parseEnumElement(data);
    EnumElements* elements = avCallocate(1, sizeof(EnumElements), "allocating enum elements");
    memcpy(&elements->element, &element, sizeof(EnumElement));

    EnumElements* next = elements;
    while (matchToken(data, TOKEN_VALUE_TYPE_COMMA)) {
        EnumElement elem = parseEnumElement(data);
        next->next = avCallocate(1, sizeof(EnumElements), "allocating enum elements");
        next = next->next;
        memcpy(&next->element, &elem, sizeof(EnumElement));
    }
    return elements;
}

static EnumElement parseEnumElement(ASTdata data) {
    Token identifier = consumeToken(data, TOKEN_IDENTIFIER, "expected identifier!");
    Expression expr = nullptr;
    if (matchToken(data, TOKEN_VALUE_TYPE_ASSIGN)) {
        expr = parseExpression(data);
    }
    return (EnumElement) {
        .identifier = identifier,
            .value = expr
    };
}

// static AccessModifier parseAccessModifier(ASTdata data) {
//     if (matchToken(data,
//         TOKEN_VALUE_TYPE_VOLATILE,
//         TOKEN_VALUE_TYPE_CONST
//     )) {
//         Token storageClass = previousToken(data);
//         switch (tokenGetType(storageClass)) {
//         case TOKEN_VALUE_TYPE_CONST:
//             return ACCESS_MODIFIER_CONST;
//             break;
//         case TOKEN_VALUE_TYPE_VOLATILE:
//             return ACCESS_MODIFIER_VOLATILE;
//             break;
//         default:
//             avAssert(0, "error");
//         }
//     }
//     return ACCESS_MODIFIER_NONE;
// }

static StorageClass parseStorageClass(ASTdata data) {
    if (matchToken(data,
        TOKEN_VALUE_TYPE_EXTERN,
        TOKEN_VALUE_TYPE_STATIC
    )) {
        Token storageClass = previousToken(data);
        switch (tokenGetType(storageClass)) {
        case TOKEN_VALUE_TYPE_EXTERN:
            return STORAGE_CLASS_EXTERN;
            break;
        case TOKEN_VALUE_TYPE_STATIC:
            return STORAGE_CLASS_STATIC;
            break;
        default:
            avAssert(0, "error");
        }
    }
    return STORAGE_CLASS_NONE;
}

static Expression parseExpression(ASTdata data) {
    return parseEquality(data);
}

static Statement parseExpressionStatement(ASTdata data) {
    Expression expr = parseExpression(data);
    consumeToken(data, TOKEN_VALUE_TYPE_SEMICOLON, "Expected ';' after expression.");
    return createStatement((Statement_T) {
        .type = STATEMENT_TYPE_EXPRESSION,
            .expression.expression = expr
    });
}

static Statement parseDefinitionStatement(ASTdata data) {
    StorageClass storageClass = parseStorageClass(data);
    Type type = parseType(data);
    Token identifier = consumeToken(data, TOKEN_VALUE_TYPE_IDENTIFIER, "expected identifier!");

    DefinitionStatement definition = {
        .storageClass = storageClass,
        .type = type,
        .identifier = identifier
    };
    if (matchToken(data, TOKEN_VALUE_TYPE_BRACKET_OPEN)) {
        definition.definitionType = DEFINITION_TYPE_FUNCTION;
        if (!checkToken(data, TOKEN_VALUE_TYPE_BRACKET_CLOSE)) {
            definition.arguments = parseArguments(data);
        }
        consumeToken(data, TOKEN_VALUE_TYPE_BRACKET_CLOSE, "expected ')'!");
        if (matchToken(data, TOKEN_VALUE_TYPE_CURLY_BRACKET_OPEN)) {
            definition.functionBody = parseBlock(data);
            definition.containsFunctionBody = true;
            consumeToken(data, TOKEN_VALUE_TYPE_CURLY_BRACKET_CLOSE, "expected '}'!");
        } else {
            consumeToken(data, TOKEN_VALUE_TYPE_SEMICOLON, "expected ';'!");
        }
    } else {
        definition.definitionType = DEFINITION_TYPE_VARIABLE;

        Expression expression = nullptr;
        if (matchToken(data, TOKEN_VALUE_TYPE_ASSIGN)) {
            expression = parseExpression(data);
        }
        consumeToken(data, TOKEN_VALUE_TYPE_SEMICOLON, "expected ';'!");
        definition.variableInitializer.expression = expression;
    }
    return createStatement((Statement_T) {
        .type = STATEMENT_TYPE_DEFINITION,
            .definition = definition
    });
}

static Statement parseReturnStatement(ASTdata data) {
    Expression expr = nullptr;
    if (!matchToken(data, TOKEN_VALUE_TYPE_SEMICOLON)) {
        expr = parseExpression(data);
        consumeToken(data, TOKEN_VALUE_TYPE_SEMICOLON, "expected ';'!");
    }
    return createStatement((Statement_T) {
        .type = STATEMENT_TYPE_RETURN,
            .returnStatement.expression = expr
    });
}

static Statement parseStatement(ASTdata data) {
    if (matchToken(data, TOKEN_VALUE_TYPE_TYPEDEF)) {
        return parseTypedefStatement(data);
    }
    if (matchToken(data, TOKEN_VALUE_TYPE_RETURN)) {
        return parseReturnStatement(data);
    }
    if (checkToken(data, TOKEN_VALUE_TYPE_IDENTIFIER)) {
        if (isType(data, peekToken(data))) {
            return parseDefinitionStatement(data);
        }
        return parseExpressionStatement(data);
    } else if ((tokenGetType(peekToken(data)) & KEYWORD_VALUE_TYPE) != 0) {
        return parseDefinitionStatement(data);
    } else {
        return parseExpressionStatement(data);
    }
}

static StatementList* parseTokens(ASTdata data) {
    StatementList* statements = avCallocate(1, sizeof(StatementList), "allocating statementList");
    statements->statement = parseStatement(data);
    StatementList* next = statements;
    while (*data.index < data.tokenCount) {
        next->next = avCallocate(1, sizeof(StatementList), "allocating statementList");
        next = next->next;
        next->statement = parseStatement(data);
    }

    return statements;
}

static void printLevel(uint32 level) {
    for (uint32 i = 0; i < level; i++) {
        printf("  ");
    }
}

static void printExpression(uint32 level, Expression expression) {

    switch (expression->type) {
    case EXPRESSION_TYPE_BINARY:
        printLevel(level);
        printf("left: {\n");
        printExpression(level + 1, expression->binary.left);
        printLevel(level);
        printf("},\n");
        printLevel(level);
        printf("op: ");
        avStringPrintln(operatorStrs[expression->binary.operator]);
        printLevel(level);
        printf("right: {\n");
        printExpression(level + 1, expression->binary.right);
        printLevel(level);
        printf("}\n");
        break;
    case EXPRESSION_TYPE_UNARY:
        printLevel(level);
        printf("unary: ");
        avStringPrint(operatorStrs[expression->unary.operator]);
        printf(" {\n");
        printExpression(level + 1, expression->unary.expression);
        printLevel(level);
        printf("}\n");
        break;
    case EXPRESSION_TYPE_LITERAL:
        printLevel(level);
        printf("value: ");
        avStringPrintLn(expression->literal.token.str);
        break;
    case EXPRESSION_TYPE_CALL:
        printLevel(level);
        printf("call: {\n");
        printExpression(level + 1, expression->call.function);
        printLevel(level);
        printf("} (");
        {
            ParameterList* parameter = expression->call.parameters;
            if (parameter) {
                printf("\n");
                while (parameter) {
                    printExpression(level + 1, parameter->param);
                    parameter = parameter->next;
                    if (parameter) {
                        printLevel(level);
                        printf(",\n");
                    }
                }
                printLevel(level);
            }
            printf(")\n");
        }
        
        printf(")\n");
        break;

    case EXPRESSION_TYPE_GROUPING:
        printExpression(level + 1, expression->grouping.content);
        break;
        default:

        break;
    }
}

static void printStatement(uint32 level, Statement statement);
static void printBlock(uint32 level, Block block) {
    StatementList* list = block.statements;
    while (list != nullptr) {
        printStatement(level, list->statement);
        list = list->next;
    }
}

static void printType(uint32 level, Type type) {
    printLevel(level);
    avStringPrint(type.identifier.str);
}

static void printStatement(uint32 level, Statement statement) {
    if (statement == nullptr) {
        return;
    }
    switch (statement->type) {
    case STATEMENT_TYPE_DEFINITION:
        printLevel(level);
        printType(level, statement->definition.type);
        printf(" ");
        avStringPrint(statement->definition.identifier.str);
        printf(" = {\n");
        switch (statement->definition.definitionType) {
        case DEFINITION_TYPE_FUNCTION:
            if (statement->definition.containsFunctionBody) {
                printBlock(level + 1, statement->definition.functionBody);
            }
            break;
        case DEFINITION_TYPE_VARIABLE:
            if (statement->definition.variableInitializer.expression) {
                printExpression(level + 1, statement->definition.variableInitializer.expression);
            }
            break;
        }
        printLevel(level);
        printf("};\n");
        break;
    case STATEMENT_TYPE_RETURN:
        printLevel(level);
        printf("return");
        if (statement->returnStatement.expression) {
            printf(" = {\n");
            printExpression(level + 1, statement->returnStatement.expression);
            printLevel(level);
            printf("}");
        }
        printf("\n");
        break;
    case STATEMENT_TYPE_EXPRESSION:
        printExpression(level, statement->expression.expression);
        break;
    default:
        break;
    }
}

static void printAST(StatementList* statements) {
    StatementList* list = statements;
    while (list != nullptr) {
        printStatement(0, list->statement);
        list = list->next;
    }
}

AvParseResult avBuildSyntaxTree(AvCompileData data) {
    // Token nullToken = {
    //     .str = AV_CSTR(""),
    //     .type = TOKEN_NONE
    // };
    //avDynamicArrayAdd(&nullToken, data->data.tokens);
    avDynamicArrayMakeContiguous(data->tokens);
    Token* tokens = avDynamicArrayGetPageDataPtr(0, data->tokens);
    uint32 tokenCount = avDynamicArrayGetSize(data->tokens);
    uint32 index = 0;

    ASTdata astData = {
        .tokens = tokens,
        .tokenCount = tokenCount,
        .index = &index,
        .typedefs = nullptr
    };
    avDynamicArrayCreate(defaultTypedefCount, sizeof(AvString), &astData.typedefs);
    for (uint32 i = 0; i < defaultTypedefCount; i++) {
        avDynamicArrayAdd(defaultTypedefs + i, astData.typedefs);
    }

    StatementList* statements = parseTokens(astData);
    printAST(statements);
    return AV_PARSE_RESULT_SUCCESS;
}