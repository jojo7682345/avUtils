#include <AvUtils/avBuilder.h>
#include <AvUtils/avFileSystem.h>
#include <AvUtils/avString.h>
#include <AvUtils/avDataStructures.h>
#include <AvUtils/avLogging.h>
#include <AvUtils/avMemory.h>
#include <AvUtils/string/avChar.h>
#include <stdio.h>

typedef enum TokenOpType {
    MATCH,
    RANGE,
}TokenOpType;

#define TOKEN_LIST \
    TOKEN(INCLUDE, MATCH, "#include")\
    TOKEN(DEFINE, MATCH, "#define")\
    TOKEN(IFN_DEF, MATCH, "#ifndef")\
    TOKEN(IF_DEF, MATCH, "#ifdef")\
    TOKEN(ELSE_DEF, MATCH, "#else")\
    TOKEN(ENDIF, MATCH, "#endif")\
    TOKEN(IF, MATCH, "if")\
    TOKEN(ELSE, MATCH, "else")\
    TOKEN(CURL_OPEN, MATCH, "{")\
    TOKEN(CURL_CLOSE, MATCH, "}")\
    TOKEN(NEWLINE, MATCH, "\\\n")\
    TOKEN(END, MATCH, ";")\
    TOKEN(QUOTE, MATCH, "\"")\
    TOKEN(LESS_THAN, MATCH, "<")\
    TOKEN(GREATER_THAN, MATCH, ">")\
    TOKEN(DIVIDE, MATCH, "/")\
    TOKEN(BRACKET_OPEN, MATCH, "(")\
    TOKEN(BRACKET_CLOSE, MATCH, ")")\
    TOKEN(STAR, MATCH, "*")\
    TOKEN(AND, MATCH, "&")\
    TOKEN(DOT, MATCH, ".")\
    TOKEN(NUMBER, RANGE, "0-9")\
    TOKEN(TEXT, RANGE, "a-z","A-Z", "_-_", "0-9")\

enum { TOKEN_TYPE_COUNTER_BASE = __COUNTER__ };
#define TOKEN_TYPE_LOCAL_COUNTER (__COUNTER__ - TOKEN_TYPE_COUNTER_BASE - 1)
#define TOKEN(token, OP, ...) TOKEN_##token = (1 << TOKEN_TYPE_LOCAL_COUNTER),
typedef enum TokenType {
    TOKEN_NONE = 0,
    TOKEN_LIST
} TokenType;
#undef TOKEN

#define TOKEN(token, OP, ...) "TOKEN_"#token,
const char* tokenTypeStrs[] = {
    TOKEN_LIST
};
#undef TOKEN

#define TOKEN(token, OP, ...) Token##token token;
typedef struct TokenFormat {
    TokenType type;
    TokenOpType op;
    uint32 valueCount;
    const char** values;
} TokenFormat;
#undef TOKEN

#define TOKEN(token, OP, ...) { .type=TOKEN_##token, .op=OP, .values =(const char*[]) {__VA_ARGS__}, .valueCount=(sizeof((const char*[]){__VA_ARGS__})/sizeof(const char*))},
const TokenFormat tokenFormats[] = {
    TOKEN_LIST
};
const uint32 tokenFormatCount = sizeof(tokenFormats) / sizeof(TokenFormat);
#undef TOKEN

typedef struct Token {
    TokenType type;
    AvString str;
} Token;

typedef struct AvCompileData_T {
    AV_DS(AvDynamicArray, Token) tokens;
}AvCompileData_T;

static bool32 isValidToken(TokenType type, AvString prev, char chr) {
    if (type == TOKEN_NONE) {
        return false;
    }
    uint32 index = 0;
    while ((type >> index) & 1 == 0) {
        index++;
    }
    TokenFormat format = tokenFormats[index];
    switch (format.op) {
    case MATCH:
        if (prev.len >= avCStringLength(format.values[0])) {
            return false;
        }
        if (format.values[0][prev.len] != chr) {
            return false;
        }
        return true;
        break;
    case RANGE:
        for (uint32 i = 0; i < format.valueCount; i++) {
            if (avCharIsWithinRange(chr, format.values[i][0], format.values[i][2])) {
                return true;
            }
        }
        return false;
        break;
    }
}
static TokenType findTokenType(char chr) {
    for (uint32 i = 0; i < tokenFormatCount; i++) {
        TokenFormat format = tokenFormats[i];
        if (isValidToken(format.type, AV_STR(nullptr, 0), chr)) {
            return format.type;
        }
    }
    return TOKEN_NONE;
}


static AvParseResult tokenize(AvString file, AvCompileData data) {

    avDynamicArrayCreate(0, sizeof(Token), &data->tokens);
    uint64 startToken = 0;
    uint64 endToken = 0;
    TokenType tokenType = TOKEN_NONE;
    for (uint64 index = 0; index < file.len; index++) {
        char chr = file.chrs[index];

        if (tokenType == TOKEN_NONE) {
            switch (chr) {
            case ' ':
            case '\t':
            case '\r':
            case '\n':
                continue;
            default:
                break;
            }
            tokenType = findTokenType(chr);
            if (tokenType == TOKEN_NONE) {
                return AV_PARSE_RESULT_INVALID_TOKEN;
            }
            startToken = index;
            endToken = index + 1;
            continue;
        }
        if (isValidToken(tokenType, AV_STR(file.chrs + startToken, endToken - startToken), chr)) {
            endToken++;
        } else {
            switch (tokenType) {
            default:
            {
                Token token = {
                    .type = tokenType,
                    .str = {
                        .chrs = file.chrs + startToken,
                        .len = endToken - startToken,
                        .memory = nullptr,
                    }
                };
                avDynamicArrayAdd(&token, data->tokens);
            }
            case TOKEN_NEWLINE:
                tokenType = TOKEN_NONE;
                startToken = index;
                index--;
                continue;
            }
        }
    }

    return AV_PARSE_RESULT_SUCCESS;
}


enum { STATEMENT_TYPE_COUNTER_BASE = __COUNTER__ };
#define STATEMENT_TYPE_LOCAL_COUNTER (__COUNTER__ - STATEMENT_TYPE_COUNTER_BASE - 1)
#define COUNTER STATEMENT_TYPE_LOCAL_COUNTER
typedef enum StatementType {
    STATEMENT_TYPE_NONE = 0,
    STATEMENT_TYPE_FUNCTION = 1 << COUNTER,
    STATEMENT_TYPE_RETURN = 1 << COUNTER,
    STATEMENT_TYPE_IF = 1 << COUNTER,
    STATEMENT_TYPE_VALUE = 1 << COUNTER,
    STATEMENT_TYPE_CALL = (1 << COUNTER) | STATEMENT_TYPE_VALUE,
    STATEMENT_TYPE_EXPRESSION = (1 << COUNTER) | STATEMENT_TYPE_VALUE,
    STATEMENT_TYPE_FILE = (1<<COUNTER),
    STATEMENT_TYPE_STRING = (1<<COUNTER) | STATEMENT_TYPE_VALUE,

    STATEMENT_TYPE_ALL = (-1)
} StatementType;
#undef COUNTER

static AvParseResult buildSyntaxTree(AvCompileData data) {
    uint32 tokenCount = avDynamicArrayGetSize(data->tokens);
    StatementType state = STATEMENT_TYPE_NONE;
    uint32 possibleStates = 0;
    for (uint32 i = 0; i < tokenCount;) {
        

        return AV_PARSE_RESULT_SYNTAX_ERROR;
    }

    return AV_PARSE_RESULT_SUCCESS;
}

AvCompileData createCompileData() {
    return avCallocate(1, sizeof(AvCompileData_T), "allocating compiledata");
}

void destroyCompileData(AvCompileData data) {
    avDynamicArrayDestroy(data->tokens);
    avFree(data);
}

AvParseResult avCompileBuildFile(AvString fileName) {
    AvCompileData data = createCompileData();

    AvParseResult result = AV_PARSE_RESULT_SUCCESS;
    AvFile file = AV_EMPTY;

    avFileHandleCreate(fileName, &file);
    if (avFileOpen(file, (AvFileOpenOptions) { .binary = true, .update = false, .openMode = AV_FILE_OPEN_READ }) == false) {
        result = AV_FILE_NOT_FOUND;
        goto fileReadExit;
    }
    AvStringMemory memory = AV_EMPTY;
    avStringMemoryAllocate(avFileGetSize(file), &memory);
    avFileRead(memory.data, memory.capacity, file);
    avFileClose(file);

    AvString fileData = AV_EMPTY;
    avStringFromMemory(&fileData, AV_STRING_WHOLE_MEMORY, &memory);

    if (!tokenize(fileData, data)) {
        result = AV_PARSE_RESULT_ERROR;
        goto parseExit;
    }


parseExit:
    avStringFree(&fileData);
fileReadExit:
    avFileHandleDestroy(file);
    destroyCompileData(data);
    return result;
}