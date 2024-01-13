#ifndef __AV_C_LANGUAGE__
#define __AV_C_LANGUAGE__

#include <AvUtils/avString.h>

enum TokenType {
    TOKEN_TYPE_NONE = 0,
    TOKEN_TYPE_KEYWORD,
    TOKEN_TYPE_IDENTIFIER,
    TOKEN_TYPE_CONSTANT,
    TOKEN_TYPE_STRING_LITERAL,
    TOKEN_TYPE_PUNCTUATOR,
};

enum Keyword {
    KEYWORD_NONE = 0,
    KEYWORD_AUTO,
    KEYWORD_BREAK,
    KEYWORD_CASE,
    KEYWORD_CHAR,
    KEYWORD_CONST,
    KEYWORD_CONTINUE,
    KEYWORD_DEFAULT,
    KEYWORD_DO,
    KEYWORD_DOUBLE,
    KEYWORD_ELSE,
    KEYWORD_ENUM,
    KEYWORD_EXTERN,
    KEYWORD_FLOAT,
    KEYWORD_FOR,
    KEYWORD_GOTO,
    KEYWORD_IF,
    KEYWORD_INLINE,
    KEYWORD_INT,
    KEYWORD_LONG,
    KEYWORD_REGISTER,
    KEYWORD_RESTRICT,
    KEYWORD_RETURN,
    KEYWORD_SHORT,
    KEYWORD_SIGNED,
    KEYWORD_SIZEOF,
    KEYWORD_STATIC,
    KEYWORD_STRUCT,
    KEYWORD_SWITCH,
    KEYWORD_TYPEDEF,
    KEYWORD_UNION,
    KEYWORD_UNSIGNED,
    KEYWORD_VOID,
    KEYWORD_VOLATILE,
    KEYWORD_WHILE,
};

enum Punctuator {
    PUNCTUATOR_NONE = 0,
    PUNCTUATOR_SQUARE_BRACKET_OPEN,
    PUNCTUATOR_SQUARE_BRACKET_CLOSE,
    PUNCTUATOR_BRACKET_OPEN,
    PUNCTUATOR_BRACKET_CLOSE,
    PUNCTUATOR_CURLY_BRACKET_OPEN,
    PUNCTUATOR_CURLY_BRACKET_CLOSE,
    PUNCTUATOR_DOT,
    PUNCTUATOR_ACCESS,
    PUNCTUATOR_INCREMENT,
    PUNCTUATOR_DECREMENT,
    PUNCTUATOR_AND,
    PUNCTUATOR_STAR,
    PUNCTUATOR_PLUS,
    PUNCTUATOR_MINUS,
    PUNCTUATOR_EXCLAMATION,
    PUNCTUATOR_SLASH,
    PUNCTUATOR_MODULO,
    PUNCTUATOR_LSHIFT,
    PUNCTUATOR_RSHIFT,
    PUNCTUATOR_LESS_THAN,
    PUNCTUATOR_GREATER_THAN,
    PUNCTUATOR_LESS_THAN_OR_EQUAL,
    PUNCTUATOR_GREATHER_THAN_OR_EQUAL,
    PUNCTUATOR_EQUALS,
    PUNCTUATOR_NOT_EQUALS,
    PUNCTUATOR_BITWISE_XOR,
    PUNCTUATOR_BITWISE_OR,
    PUNCTUATOR_LOGICAL_AND,
    PUNCTUATOR_LOGICAL_OR,
    PUNCTUATOR_QUESTION_MARK,
    PUNCTUATOR_COLON,
    PUNCTUATOR_SEMICOLON,
    PUNCTUATOR_DOTDOTDOT,
    PUNCTUATOR_ASSIGN,
    PUNCTUATOR_MUL_ASSIGN,
    PUNCTUATOR_DIV_ASSIGN,
    PUNCTUATOR_MOD_ASSIGN,
    PUNCTUATOR_ADD_ASSIGN,
    PUNCTUATOR_SUB_ASSIGN,
    PUNCTUATOR_LSH_ASSIGN,
    PUNCTUATOR_RSH_ASSIGN,
    PUNCTUATOR_AND_ASSIGN,
    PUNCTUATOR_XOR_ASSIGN,
    PUNCTUATOR_OR_ASSIGN,
    PUNCTUATOR_COMMA,
    PUNCTUATOR_HASH,
    PUNCTUATOR_HASH_HASH,
};

struct Identifier {
    AvString str;
};

enum ConstantType {
    CONSTANT_TYPE_NONE = 0,
    CONSTANT_TYPE_INTEGER,
    CONSTANT_TYPE_FLOAT,
    CONSTANT_TYPE_ENUMERATION,
    CONSTANT_TYPE_CHARACTER,
};

enum IntegerConstantType {
    INTEGER_CONSTANT_TYPE_NONE = 0,
    INTEGER_CONSTANT_TYPE_DECIMAL,
    INTEGER_CONSTANT_TYPE_OCTAL,
    INTEGER_CONSTANT_TYPE_HEXADECIMAL,
    INTEGER_CONSTANT_TYPE_BINARY,
};

struct DecimalConstant {
    AvString str;
};

struct OctalConstant {
    AvString str;
};

struct HexadecimalConstant {
    AvString str;
};

struct BinaryConstant {
    AvString str;
};

enum IntegerSuffix {
    INTEGER_SUFFIX_NONE = 0,
    INTEGER_SUFFIX_UNSIGNED = 0x1,
    INTEGER_SUFFIX_LONG = 0x2,
    INTEGER_SUFFIX_LONG_LONG = 0x6,
};

struct IntegerConstant {
    enum IntegerConstantType type;
    union {
        struct DecimalConstant decimalConstant;
        struct OctalConstant octalConstant;
        struct HexadecimalConstant hexadecimalConstant;
        struct BinaryConstant binaryConstant;
    };
    enum IntegerSuffix integerSuffix;
};

struct FractionalConstant {
    AvString integer;
    AvString fraction;
};

struct ExponentPart {
    bool32 sign; // 1 for negative 0 for positive;
    AvString digits;
};

enum FloatingSuffix {
    FLOATING_SUFFIX_DOUBLE = 0,
    FLOATING_SUFFIX_FLOAT = 1,
};

struct FloatingConstant {
    struct FractionalConstant fractionalConstant;
    struct ExponentPart exponentPart;
    enum FloatingSuffix floatingSuffix;
};

struct EnumerationConstant {
    AvString identifier;
};

enum EncodingPrefix {
    ENCODING_PREFIX_NONE = 0,
    ENCODING_PREFIX_UNSIGNED = 0x1,
    ENCODING_PREFIX_WIDE = 0x2,
};

struct CharacterConstant {
    enum EncodingPrefix specifier;
    AvString charSequence;
};

struct Constant {
    enum ConstantType type;
    union {
        struct IntegerConstant integerConstant;
        struct FloatingConstant floatingConstant;
        struct EnumerationConstant enumerationConstant;
        struct CharacterConstant characterConstant;
    };
};

struct StringLiteral {
    enum EncodingPrefix encodingPrefix;
    AvString sequence;
};

struct Token {
    enum TokenType type;
    union {
        enum Keyword keyword;
        struct Identifier identifier;
        struct Constant constant;
        struct StringLiteral stringLiteral;
        enum Punctuator punctuator;
    };
};

enum PreprocessorTokenType {
    PREPROCESSOR_TOKEN_TYPE_NONE = 0,
    PREPROCESSOR_TOKEN_TYPE_HEADER_NAME,
    PREPROCESSOR_TOKEN_TYPE_PP_NUMBER,
    PREPROCESSOR_TOKEN_TYPE_CHARACTER_CONSTANT,
    PREPROCESSOR_TOKEN_TYPE_STRING_LITERAL,
    PREPROCESSOR_TOKEN_TYPE_PUNCTUATOR,
    PREPROCESSOR_TOKEN_TYPE_ANY,
};

struct HeaderName {
    AvString str;
};

struct PPnumber {
    AvString str;
};

struct PreprocessorToken {
    enum PreprocessorTokenType type;
    union {
        struct HeaderName headerName;
        struct Identifier identifier;
        struct PPnumber ppNumber;
        struct CharacterConstant characterConstant;
        struct StringLiteral stringLiteral;
        enum Punctuator punctuator;
        AvString str;
    };
};

enum CharacterSpecifier {
    CHARACTER_SPECIFIER_NONE = 0,
    CHARACTER_SPECIFIER_UNSIGNED = 0x1,
    CHARACTER_SPECIFIER_WIDE = 0x2,
};



enum PrimaryExpressionType {
    PRIMARY_EXPRESSION_TYPE_NONE = 0,
    PRIMARY_EXPRESSION_TYPE_IDENTIFIER,
    PRIMARY_EXPRESSION_TYPE_CONSTANT,
    PRIMARY_EXPRESSION_TYPE_STRING_LITERAL,
    PRIMARY_EXPRESSION_TYPE_NESTED_EXPRESSION,
};

struct PrimaryExpression {
    enum PrimaryExpressionType type;
    union {
        struct Identifier identifier;
        struct Constant constant;
        struct StringLiteral stringLiteral;
        struct Expression* nestedExpression;
    };
};

enum PostfixExpressionType {
    POSTFIX_EXPRESSION_TYPE_NONE = 0,
    POSTFIX_EXPRESSION_TYPE_PRIMARY,
    POSTFIX_EXPRESSION_TYPE_INDEX,
    POSTFIX_EXPRESSION_TYPE_CALL,
    POSTFIX_EXPRESSION_TYPE_ACCESS,
    POSTFIX_EXPRESSION_TYPE_INCREMENT,
    POSTFIX_EXPRESSION_TYPE_DECREMENT,
    POSTFIX_EXPRESSION_TYPE_COMPOUND_LITERAL,
};

enum TypeQualifier {
    TYPE_QUALIFIER_NONE = 0,
    TYPE_QUALIFIER_CONST,
    TYPE_QUALIFIER_RESTRICT,
    TYPE_QUALIFIER_VOLATILE,
};

enum TypeSpecifierType {
    TYPE_SPECIFIER_NONE = 0,
    TYPE_SPECIFIER_VOID,
    TYPE_SPECIFIER_CHAR,
    TYPE_SPECIFIER_SHORT,
    TYPE_SPECIFIER_INT,
    TYPE_SPECIFIER_LONG,
    TYPE_SPECIFIER_FLOAT,
    TYPE_SPECIFIER_DOUBLE,
    TYPE_SPECIFIER_SIGNED,
    TYPE_SPECIFIER_UNSIGNED,
    TYPE_SPECIFIER_STRUCT_OR_UNION,
    TYPE_SPECIFIER_ENUM,
    TYPE_SPECIFIER_TYPEDEF_NAME,
};

enum StructOrUnion {
    STRUCT,
    UNION,
};

struct StructOrUnionSpecifier {
    enum StructOrUnion structOrUnion;
    struct Identifier identifier;
    // {
    struct StructDeclarationList* structDeclarationList;
    // }
};

struct TypedefName {
    struct Identifier identifier;
};

struct EnumSpecifier {
    // enum
    struct Identifier identifier;
    // {
    struct EnumeratorList* enumeratorList;
    // }
};

struct TypeSpecifier {
    enum TypeSpecifierType type;
    union {
        struct StructOrUnionSpecifier structOrUnionSpecifier;
        struct EnumSpecifier enumSpecifier;
        struct TypedefName typedefName;
    };
};

struct SpecifierQualifierList {
    enum TypeQualifier typeQualifier;
    struct TypeSpecifier typeSpecifier;
    struct SpecifierQualifierList* specifierQualifierList;
};

struct TypeName {
    struct SpecifierQualifierList specifierQualifierList;
    struct AbstractDeclarator* abstractDeclarator;
};

enum InitializerType {
    INITIALIZER_TYPE_EXPRESSION,
    INITIALIZER_TYPE_INITIALIZER_LIST,
};

enum AssignmentExpressionType {
    ASSIGNMENT_EXPRESSION_TYPE_NONE = 0,
    ASSIGNMENT_EXPRESSION_TYPE_CONDITIONAL,
    ASSIGNMENT_EXPRESSION_TYPE_ASSIGNMENT,
};

enum ConditionalExpressionType {
    CONDITIONAL_EXPRESSION_TYPE_NONE = 0,
    CONDITIONAL_EXPRESSION_TYPE_LOGICAL_OR,
    CONDITIONAL_EXPRESSION_TYPE_CONDITIONAL,
};

struct ConditionalExpression {
    enum ConditionalExpressionType type;
    struct LogicalOrExpression* condition;
    struct Expression* trueExpression;
    struct ConditionalExpression* falseExpression;
};

struct AssignmentExpression {
    enum AssignmentExpressionType type;
    union {
        struct ConditionalExpression conditionalExpression;
        struct {
            struct UnaryExpression* unaryExpression;
            enum Punctuator operator;
            struct AssignmentExpression* assignmentExpression;
        }assignment;
    };
};

struct Initializer {
    enum InitializerType type;
    union {
        struct AssignmentExpression assignmentExpression;
        struct InitializerList* initializerList;
    };
};

struct InitializerList {
    struct Designation* designation;
    struct Initializer initializer;
    struct InitializerList* initializerList;
};

struct PostFixExpression {
    enum PostfixExpressionType type;
    union {
        struct PrimaryExpression* primaryExpression;
        struct {
            struct PostFixExpression* postFixExpression;
            struct Expression* index;
        }index;
        struct {
            struct PostFixExpression* postFixExpression;
            struct ArgumentExpressionList* argumentExpressionList;
        }call;
        struct {
            struct PostFixExpression* postFixExpression;
            enum Punctuator accessType;
            struct Identifier identifier;
        }access;
        struct {
            struct PostFixExpression* postFixExpression;
            enum Punctuator operator;
        }operator;
        struct {
            struct TypeName typeName;
            struct InitializerList initializerList;
        }compountLiteral;
    };
};

struct ArgumentExpressionList {
    struct AssignmentExpression* arugmentExpressionList;
    struct AssignmentExpression assignmentExpression;
};

enum UnaryExpressionType {
    UNARY_EXPRESSION_TYPE_NONE = 0,
    UNARY_EXPRESSION_TYPE_POSTFIX_EXPRESSION,
    UNARY_EXPRESSION_TYPE_INCREMENT,
    UNARY_EXPRESSION_TYPE_DECREMENT,
    UNARY_EXPRESSION_TYPE_OPERATOR,
    UNARY_EXPRESSION_TYPE_SIZEOF_EXPRESSION,
    UNARY_EXPRESSION_TYPE_SIZEOF_TYPE,
};

struct UnaryExpression {
    enum UnaryExpressionType type;
    union {
        struct PostFixExpression* postFixExpression;
        struct {
            enum Punctuator operator;
            struct UnaryExpression* unaryExpression;
        }operator;
        struct {
            enum Punctuator operator;
            struct CastExpression* castExpression;
        }unary;
        struct UnaryExpression* unaryExpression;
        struct TypeName typeName;
    };
};

enum CastExpressionType {
    CAST_EXPRESSION_TYPE_NONE = 0,
    CAST_EXPRESSION_TYPE_UNARY,
    CAST_EXPRESSION_TYPE_CAST,
};

struct CastExpression {
    enum CastExpressionType type;
    union {
        struct UnaryExpression* unaryExpression;
        struct {
            struct TypeName typeName;
            struct CastExpression* castExpression;
        }cast;
    };
};

enum MultiplicativeExpressionType {
    MULTIPLICATIVE_EXPRESSION_TYPE_NONE = 0,
    MULTIPLICATIVE_EXPRESSION_TYPE_CAST,
    MULTIPLICATIVE_EXPRESSION_TYPE_MULTIPLY,
    MULTIPLICATIVE_EXPRESSION_TYPE_DIVIDE,
    MULTIPLICATIVE_EXPRESSION_TYPE_MODULO,
};

struct MultiplicativeExpression {
    enum MultiplicativeExpressionType type;
    struct CastExpression* right;
    struct MultiplicativeExpression* left;
    enum Punctuator operator;
};

enum AddativeExpressionType {
    ADDATIVE_EXPRESSION_TYPE_NONE = 0,
    ADDATIVE_EXPRESSION_TYPE_MULTIPLICATIVE,
    ADDATIVE_EXPRESSION_TYPE_ADD,
    ADDATIVE_EXPRESSION_TYPE_SUB,
};

struct AddativeExpression {
    enum AddativeExpressionType type;
    struct MultiplicativeExpression* right;
    struct AddativeExpression* left;
    enum Punctuator operator;
};

enum ShiftExpressionType {
    SHIFT_EXPRESSION_TYPE_NONE = 0,
    SHIFT_EXPRESSION_TYPE_ADDATIVE,
    SHIFT_EXPRESSION_TYPE_LEFTSHIFT,
    SHIFT_EXPRESSION_TYPE_RIGHTSHIFT,
};

struct ShiftExpression {
    enum ShiftExpressionType type;
    struct AddativeExpression* right;
    struct ShiftExpression* left;
    enum Punctuator operator;
};

enum RelationalExpressionType {
    RELATIONAL_EXPRESSION_TYPE_NONE = 0,
    RELATIONAL_EXPRESSION_TYPE_LESS_THAN,
    RELATIONAL_EXPRESSION_TYPE_LESS_THAN_OR_EQUAL,
    RELATIONAL_EXPRESSION_TYPE_GREATER_THAN,
    RELATIONAL_EXPRESSION_TYPE_GREATER_THAN_OR_EQUAL,
};

struct RelationalExpression {
    enum RelationalExpressionType type;
    union {
        struct ShiftExpression* right;
        struct RelationalExpression* left;
        enum Punctuator operator;
    };
};

enum EqualityExpressionType {
    EQUALITY_EXPRESSION_TYPE_NONE = 0,
    EQUALITY_EXPRESSION_TYPE_RELATIONAL,
    EQUALITY_EXPRESSION_TYPE_EQUALS,
    EQUALITY_EXPRESSION_TYPE_NOT_EQUALS,
};

struct EqualityExpression {
    enum EqualityExpressionType type;
    union {
        struct RelationalExpression* right;
        struct EqualityExpression* left;
        enum Punctuator operator;
    };
};

enum AndExpressionType {
    AND_EXPRESSION_TYPE_NONE = 0,
    AND_EXPRESSION_TYPE_EQUALITY,
    AND_EXPRESSION_TYPE_AND,
};

struct AndExpression {
    enum AndExpressionType type;
    union {
        struct EqualityExpression* right;
        struct AndExpression* left;
        enum Punctuator operator;
    };
};

enum XorExpressionType {
    XOR_EXPRESSION_TYPE_NONE = 0,
    XOR_EXPRESSION_TYPE_AND,
    XOR_EXPRESSION_TYPE_XOR,
};

struct XorExpression {
    enum XorExpressionType type;
    union {
        struct AndExpression* right;
        struct XorExpression* left;
        enum Punctuator operator;
    };
};

enum OrExpressionType {
    OR_EXPRESSION_TYPE_NONE = 0,
    OR_EXPRESSION_TYPE_XOR,
    OR_EXPRESSION_TYPE_OR,
};

struct OrExpression {
    enum OrExpressionType type;
    union {
        struct XorExpression* right;
        struct OrExpression* left;
        enum Punctuator operator;
    };
};

enum LogicalAndExpressionType {
    LOGICAL_AND_EXPRESSION_TYPE_NONE = 0,
    LOGICAL_AND_EXPRESSION_TYPE_OR,
    LOGICAL_AND_EXPRESSION_TYPE_LOGICAL_AND,
};

struct LogicalAndExpression {
    enum LogicalAndExpressionType type;
    union {
        struct OrExpression* right;
        struct LogicalAndExpression* left;
        enum Punctuator operator;
    };
};

enum LogicalOrExpressionType {
    LOGICAL_OR_EXPRESSION_TYPE_NONE = 0,
    LOGICAL_OR_EXPRESSION_TYPE_LOGICAL_AND,
    LOGICAL_OR_EXPRESSION_TYPE_LOGICAL_OR,
};

struct LogicalOrExpression {
    enum LogicalOrExpressionType type;
    union {
        struct LogicalAndExpression* right;
        struct LogicalOrExpression* left;
        enum Punctuator operator;
    };
};









enum ExpressionType {
    EXPRESSION_TYPE_NONE = 0,
    EXPRESSION_TYPE_ASSIGNMENT,
    EXPRESSION_TYPE_MULTI_ASSIGN,
};

struct Expression {
    enum ExpressionType type;
    struct AssignmentExpression* assignmentExpression;
    struct Expression* expression;
};

struct ConstantExpression {
    struct ConditionalExpression* conditionalExpression;
};

enum DeclarationSpecifierType {
    STORAGE_CLASS_SPECIFIER,
    TYPE_SPECIFIER,
    TYPE_QUALIFIER,
    FUNCTION_SPECIFIER,
};

enum StorageClassSpecifier {
    STORAGE_CLASS_TYPEDEF,
    STORAGE_CLASS_EXTERN,
    STORAGE_CLASS_STATIC,
    STORAGE_CLASS_AUTO,
    STORAGE_CLASS_REGISTER,
};

enum FunctionSpecifier {
    FUNCTION_SPECIFIER_NONE = 0,
    FUNCTION_SPECIFIER_INLINE,
};

struct DeclarationSpecifiers {
    enum DeclarationSpecifierType type;
    union {
        enum StorageClassSpecifier storageClassSpecifier;
        struct TypeSpecifier typeSpecifier;
        enum TypeQualifier typeQualifier;
        enum FunctionSpecifier functionSpecifier;
    };
    struct DeclarationSpecifier* specifiers;
};

struct Declaration {
    struct DeclarationSpecifiers declarationSpecifiers;
    struct InitDeclaratorList* initDeclaratorList;
    // ;
};





struct InitDeclaratorList {
    struct InitDeclaratorList* initDeclaratorList;
    // =
    struct InitDeclarator* initDeclarator;
};

enum DirectDeclaratorType {
    DIRECT_DECLARATOR_TYPE_NONE = 0,
    DIRECT_DECLARATOR_TYPE_IDENTIFIER,
    DIRECT_DECLARATOR_TYPE_NESTED_DECLARATOR,
    DIRECT_DECLARATOR_TYPE_ARRAY,
    DIRECT_DECLARATOR_TYPE_PARAMETERS,
    DIRECT_DECLARATOR_TYPE_IDENTIFIER_LIST,
};

struct DirectDeclarator {
    enum DirectDeclaratorType type;
    union {
        struct Identifier identifier;
        struct Declarator* declarator;
        struct {
            struct DirectDeclarator* directDeclarator;
            struct TypeQualifierList* typeQualifierList;
            struct AssignmentExpression* assignmentExpression;
        }assignment;
        struct {
            struct DirectDeclarator* directDeclarator;
            struct ParameterTypeList* parameterTypeList;
        } parameters;
        struct {
            struct DirectDeclarator* directDeclarator;
            struct IdentifierList* identifierList;
        }identifiers;
    };
};

struct Declarator {
    struct Pointer* pointer;
    struct DirectDeclarator directDeclarator;
};

struct InitDeclarator {
    struct Declarator declarator;
    // =
    struct Initializer* initializer;
};






struct StructDeclaration {
    struct SpecifierQualifierList specifierQualifierList;
    struct StructDeclaratorList* structDeclaratorList;
};

struct StructDeclarationList {
    struct StructDeclarationList* structDeclarationList;
    struct StructDeclaration structDeclaration;
};



struct StructDeclarator {
    struct Declarator* declarator;
    // :
    struct ConstantExpression* constantExpression;
};

struct StructDeclaratorList {
    struct StructDeclarator structDeclarator;
    // ,
    struct StructDeclaratorList* structDeclaratorList;
};



struct Enumerator {
    struct EnumerationConstant enumerationConstant;
    // equals
    struct ConstantExpression* constantExpression;
};

struct EnumeratorList {
    struct Enumerator enumerator;
    // comma
    struct EnumeratorList* enumeratorList;
};













struct Pointer {
    struct TypeQualifierList* typeQualifier;
    struct Pointer* pointer;
};

struct TypeQualifierList {
    enum TypeQualifier typeQualifier;
    struct TypeQualifierList* typeQualifierList;
};

enum ParameterDeclarationType {
    PARAMETER_DECLARATION_TYPE_NONE = 0,
    PARAMETER_DECLARATION_TYPE_DECLARATOR,
    PARAMETER_DECLARATION_TYPE_ABSTRACT,
};

struct ParameterDeclaration {
    struct DeclarationSpecifiers declarationSpecifiers;
    enum ParameterDeclarationType type;
    union {
        struct Declarator declarator;
        struct AbstractDeclarator* abstractDeclarator;
    };
};

struct ParameterList {
    struct ParameterDeclaration parameterDeclaration;
    struct ParameterList* parameterList;
};

struct ParameterTypeList {
    struct ParameterList parameterList;
    bool32 vaArgs;
};





struct IdentifierList {
    struct Identifier identifier;
    struct IdentifierList* identifierList;
};



struct AbstractDeclarator {
    struct Pointer* pointer;
    struct AbstractDeclarator* abstractDeclarator;
};

// TODO: direct abstract declarator


enum DesignatorType {
    DESIGNATOR_TYPE_NONE = 0,
    DESIGNATOR_TYPE_CONST_EXPR,
    DESIGNATOR_TYPE_IDENTIFIER,
};

struct Designator {
    enum DesignatorType type;
    union {
        struct ConstantExpression constantExpression;
        struct Identifier identifier;
    };
};

struct DesignatorList {
    struct Designator designator;
    struct DesignatorList* designatorList;
};



struct Designation {
    struct DesignatorList designatorList;
};



enum StatementType {
    STATEMENT_TYPE_NONE = 0,
    STATEMENT_TYPE_LABELED,
    STATEMENT_TYPE_COMPOUND,
    STATEMENT_TYPE_EXPRESSION,
    STATEMENT_TYPE_SELECTION,
    STATEMENT_TYPE_ITERATION,
    STATEMENT_TYPE_JUMP_STATEMENT,
};



enum LabeledStatementType {
    LABELED_STATEMENT_TYPE_NONE = 0,
    LABELED_STATEMENT_TYPE_LABEL,
    LABELED_STATEMENT_TYPE_CASE,
    LABELED_STATEMENT_TYPE_DEFAULT,
};

struct LabeledStatement {
    enum LabeledStatementType type;
    struct Statement* statement;
    union {
        struct Identifier identifier;
        struct ConstantExpression constantExpression;
    };
};

struct CompoundStatement {
    struct BlockItemList* blockItemList;
};

enum BlockItemType {
    BLOCK_ITEM_TYPE_NONE = 0,
    BLOCK_ITEM_TYPE_DECLARATION,
    BLOCK_ITEM_TYPE_STATEMENT,
};

struct BlockItem {
    enum BlockItemType type;
    union {
        struct Declaration declaration;
        struct Statement* statement;
    };
};

struct BlockItemList {
    struct BlockItem blockItem;
    struct BlockItemList* blockItemList;
};

struct ExpressionStatement {
    struct Expression* expression;
};

enum SelectionStatementType {
    SELECTION_STATEMENT_TYPE_NONE = 0,
    SELECTION_STATEMENT_TYPE_IF,
    SELECTION_STATEMENT_TYPE_IF_ELSE,
    SELECTION_STATEMENT_TYPE_SWITCH,
};

struct SelectionStatement {
    enum SelectionStatementType type;
    struct Expression expression;
    struct Statement* statement;
    struct Statement* elseStatement;
};

enum IterationStatementType {
    ITERATION_STATEMENT_TYPE_NONE = 0,
    ITERATION_STATEMENT_TYPE_WHILE,
    ITERATION_STATEMNET_TYPE_DO_WHILE,
    ITERATION_STATEMENT_TYPE_FOR,
    ITERATION_STATEMENT_TYPE_FOR_ALT,
};

struct IterationStatement {
    enum IterationStatementType type;
    struct Statement* statement;
    union {
        struct {
            struct Expression expression;

        } _while;
        struct {
            struct Expression* varDeclaration;
            struct Expression* limitDeclaration;
            struct Expression* increment;
            struct Declaration* declaration;
        } _for;
    };
};

enum JumpstatementType {
    JUMP_STATEMENT_TYPE_NONE = 0,
    JUMP_STATEMENT_TYPE_GOTO,
    JUMP_STATEMENT_TYPE_CONTINUE,
    JUMP_STATEMENT_TYPE_BREAK,
    JUMP_STATEMENT_TYPE_RETURN,
};

struct JumpStatement {
    enum JumpstatementType type;
    struct Identifier* identifier;
    struct Expression* expression;
};

struct Statement {
    enum StatementType type;
    union {
        struct LabeledStatement labeledStatement;
        struct CompoundStatement compoundStatement;
        struct ExpressionStatement expressionStatement;
        struct SelectionStatement selectionStatement;
        struct IterationStatement iterationStatement;
        struct JumpStatement jumpStatement;
    };
};

enum ExternalDeclarationType {
    EXTERNAL_DECLARATION_TYPE_NONE = 0,
    EXTERNAL_DECLARATION_TYPE_FUNCTION_DEFINITION,
    EXTERNAL_DECLARATION_TYPE_DECLARATION,
};

struct FunctionDefinition {
    struct DeclarationSpecifiers declarationSpecifiers;
    struct Declarator declarator;
    struct DeclarationList* declarationList;
    struct CompoundStatement compoundStatement;
};

struct DeclarationList {
    struct Declaration declaration;
    struct DeclarationList* declarationList;
};

struct ExternalDeclaration {
    enum ExternalDeclarationType type;
    union {
        struct FunctionDefinition functionDefinition;
        struct Declaration declaration;
    };
};

struct TranslationUnit {
    struct ExternalDeclaration externalDeclaration;
    struct TranslationUnit* translationUnit;
};

#endif//__AV_C_LANGUAGE__

