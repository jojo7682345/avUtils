#include "avTokenizer.h"
#include <AvUtils/string/avChar.h>
#include <AvUtils/avLogging.h>
#include <stdio.h>

#define TOKEN(token, OP, ...) "TOKEN_"#token,
const char* tokenTypeStrs[] = {
    TOKEN_LIST
};
#undef TOKEN

#define PRE_PROCESSOR(name, str)  "#"str,
#define KEYWORD(name, str)  str,
#define SPECIAL(name, str)  str,
#define OPERATOR(name, str)  str,
#define TOKEN(token, OP, ...) { .type=TOKEN_##token, .op=OP, .values =(const char*[]) {__VA_ARGS__}, .valueCount=(sizeof((const char*[]){__VA_ARGS__})/sizeof(const char*))},
const TokenFormat tokenFormats[] = {
    TOKEN_LIST
};
const uint32 tokenFormatCount = sizeof(tokenFormats) / sizeof(TokenFormat);
#undef TOKEN
#undef OPERATOR
#undef SPECIAL
#undef KEYWORD
#undef PRE_PROCESSOR

static bool32 isValidToken(TokenType type, AvString prev, char chr) {
    if (type == TOKEN_NONE) {
        return false;
    }
    uint32 index = 0;
    while (((type >> index) & 1) == 0) {
        index++;
    }
    TokenFormat format = tokenFormats[index];
    switch (format.op) {
    case MATCH:
        for (uint i = 0; i < format.valueCount; i++) {
            if (prev.len >= avCStringLength(format.values[i])) {
                continue;
            }
            if(!avStringEquals(prev, AV_STR(format.values[i],prev.len))){
                continue;
            }
            if (format.values[i][prev.len] != chr) {
                continue;
            }
            return true;
        }
        return false;
    case RANGE:
        for (uint32 i = 0; i < format.valueCount; i++) {
            if (avCharIsWithinRange(chr, format.values[i][0], format.values[i][2])) {
                return true;
            }
        }
        return false;
        break;
    case NUMBER:
        if (prev.len >= 2) {
            if (prev.chrs[0] == '0') {
                if (prev.chrs[1] == 'x') {
                    if (avCharIsHexNumber(chr)) {
                        return true;
                    }
                    return false;
                }
                if (prev.chrs[1] == 'b') {
                    if (avCharIsWithinRange(chr, '0', '1')) {
                        return true;
                    }
                    return false;
                }
                if (prev.chrs[1] != '.') {
                    return false;
                }
            }
        }
        if (prev.len == 1) {
            if (prev.chrs[0] == '0') {
                if (chr == 'x') {
                    return true;
                }
                if (chr == 'b') {
                    return true;
                }
            }
        }
        if (chr == '.') {
            if (avStringFindCharCount(prev, '.') == 0) {
                return true;
            }
            return false;
        }
        if (chr == 'f') {
            if (avStringFindCharCount(prev, 'f') == 0) {
                return true;
            }
            return false;
        }
        if (avCharIsNumber(chr)) {
            return true;
        }
        return false;
    case STRING:
        if (prev.len > 0) {
            if (chr == '"') {
                if (prev.len > 1 && prev.chrs[prev.len - 1] == '"') {
                    if (prev.len >= 2 && prev.chrs[prev.len - 2] == '\\') {
                        return true;
                    }
                    return false;
                }
            }
            if (prev.len >= 2) {
                if (prev.chrs[prev.len - 1] == '"') {
                    if (prev.chrs[prev.len - 2] != '\\') {
                        return false;
                    }
                }
            }
            switch (chr) {
            case '\n':
                if (prev.chrs[prev.len - 1] == '\\') {
                    return true;
                }
                if (prev.chrs[prev.len - 1] == '\r' && prev.len >= 2 && prev.chrs[prev.len - 2] == '\\') {
                    return true;
                }
            case '\f':
                return false;
            default:
                return true;
            }
        }

        if (chr == '"') {
            return true;
        }
        return false;
    }
    return false;
}
static TokenType findTokenType(TokenType possibleTokens, AvString prev, char chr) {
    if (possibleTokens == TOKEN_NONE) {
        for (uint32 i = 0; i < tokenFormatCount; i++) {
            TokenFormat format = tokenFormats[i];
            if (isValidToken(format.type, AV_STR(nullptr, 0), chr)) {
                possibleTokens |= format.type;
            }
        }
        return possibleTokens;
    }
    uint32 index = 0;
    TokenType type = possibleTokens;
    while (type != TOKEN_NONE) {
        while (((type >> index) & 1) == 0) {
            index++;
        }
        if (!isValidToken(1 << index, prev, chr)) {
            possibleTokens &= ~(1 << index);
        }
        type &= ~(1 << index);
    }
    return possibleTokens;

}

static void buildToken(TokenType* tokenType, const char* chrs, uint64* startToken, uint64 endToken, AvDynamicArray tokens, uint64* index){
    uint32 bit = 0;
    switch (*tokenType) {
    default:
    {
        while (((*tokenType >> bit) & 1) == 0) {
            bit++;
        }
        Token token = {
            .type = 1 << bit,
            .str = {
                .chrs = chrs + *startToken,
                .len = endToken - *startToken,
                .memory = nullptr,
            }
        };
        avDynamicArrayAdd(&token, tokens);
    }
    case TOKEN_NEWLINE:
        *tokenType = TOKEN_NONE;
        *startToken = *index;
        (*index)--;
        return;
    case TOKEN_NONE:
        return;
    }
}

AvParseResult avTokenize(AvString file, AvCompileData data) {

    avDynamicArrayCreate(0, sizeof(Token), &data->tokens);
    uint64 startToken = 0;
    uint64 endToken = 0;
    TokenType tokenType = TOKEN_NONE;
    TokenType prevToken = TOKEN_NONE;
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
            tokenType = findTokenType(tokenType, AV_STR(file.chrs + startToken, endToken - startToken), chr);
            if (tokenType == TOKEN_NONE) {
                return AV_PARSE_RESULT_INVALID_TOKEN;
            }
            startToken = index;
            endToken = index;
            prevToken = tokenType;
        }

        uint32 bit = 0;
        TokenType type = tokenType;
        while (type != TOKEN_NONE) {
            while (((type >> bit) & 1) == 0) {
                bit++;
            }
            if (!isValidToken(1 << bit, AV_STR(file.chrs + startToken, endToken - startToken), chr)) {
                tokenType &= ~(1 << bit);
            }
            type &= ~(1 << bit);
        }
        if (tokenType != TOKEN_NONE) {
            endToken++;
            prevToken = tokenType;
        } else {
            buildToken(&prevToken, file.chrs, &startToken, endToken, data->tokens, &index);
        }
    }
    uint64 lastIndex = file.len;
    buildToken(&prevToken, file.chrs, &startToken, endToken, data->tokens, &lastIndex);

    for (uint32 index = 0; index < avDynamicArrayGetSize(data->tokens); index++) {
        Token element; avDynamicArrayRead(&element, index, (data->tokens));
        avStringPrint(element.str);
        uint32 bit = 0;
        while (((element.type >> bit) & 1) == 0) {
            bit++;
        }
        printf(" -> %s -> %x\n", tokenTypeStrs[bit], element.type);
        avAssert(element.type >> (bit + 1) == 0, "invalid token type");
    }

    return AV_PARSE_RESULT_SUCCESS;
}


#define TYPEDEF(name, str) {.chrs=str, .len=sizeof(str)/sizeof(char)-1,.memory=nullptr},
AvString defaultTypedefs[] = {
    DEFAULT_TYPELIST
};
const uint32 defaultTypedefCount = sizeof(defaultTypedefs) / sizeof(AvString);
#undef TYPEDEF

#define PRE_PROCESSOR(name, str) {.chrs=str,.len=sizeof(str)/sizeof(char)-1, .memory=AV_STRING_CONST},
AvString preprocessorStrs[] = {
    PRE_PROCESSOR_LIST
};
const uint32 preprocessorStrCount = sizeof(preprocessorStrs) / sizeof(AvString);
#undef PRE_PROCESSOR
#define KEYWORD(name, str) {.chrs=str,.len=sizeof(str)/sizeof(char)-1, .memory=AV_STRING_CONST},
AvString keywordStrs[] = {
    KEYWORD_LIST
};
const uint32 keywordStrCount = sizeof(keywordStrs) / sizeof(AvString);
#undef KEYWORD

#undef PRE_PROCESSOR
#define SPECIAL(name, str) {.chrs=str,.len=sizeof(str)/sizeof(char)-1, .memory=AV_STRING_CONST},
AvString specialStrs[] = {
    SPECIAL_LIST
};
const uint32 specialStrCount = sizeof(specialStrs) / sizeof(AvString);
#undef SPECIAL
#define OPERATOR(name, str) {.chrs=str,.len=sizeof(str)/sizeof(char)-1, .memory=AV_STRING_CONST},
AvString operatorStrs[] = {
    OPERATOR_LIST
};
const uint32 operatorStrCount = sizeof(operatorStrs) / sizeof(AvString);
#undef OPERATOR

TokenValueType tokenGetType(Token token) {
    switch (token.type) {
    case TOKEN_NONE:
        return TOKEN_VALUE_TYPE_NONE;
    case TOKEN_IDENTIFIER:
        return TOKEN_VALUE_TYPE_IDENTIFIER;
    case TOKEN_STRING:
        return TOKEN_VALUE_TYPE_STRING_LITERAL;
    case TOKEN_NUMBER:
        return TOKEN_VALUE_TYPE_NUMBER_LITERAL;

    case TOKEN_PRE_PROCESSOR:
        for (uint32 i = 0; i < preprocessorStrCount; i++) {
            if (avStringEquals(token.str, preprocessorStrs[i])) {
                return TOKEN_VALUE__PRE_PROCESSOR + ((i + 1) << 4);
            }
        }
        avAssert(token.type == 0, "token invalid");
        return TOKEN_VALUE_TYPE_NONE;
    case TOKEN_KEYWORD:
        for (uint32 i = 0; i < keywordStrCount; i++) {
            if (avStringEquals(token.str, keywordStrs[i])) {
                return TOKEN_VALUE__KEYWORD + ((i + 1) << 4);
            }
        }
        avAssert(token.type == 0, "token invalid");
        return TOKEN_VALUE_TYPE_NONE;
    case TOKEN_SPECIAL:
        for (uint32 i = 0; i < specialStrCount; i++) {
            if (avStringEquals(token.str, specialStrs[i])) {
                return TOKEN_VALUE__SPECIAL + ((i + 1) << 4);
            }
        }
        avAssert(token.type == 0, "token invalid");
        return TOKEN_VALUE_TYPE_NONE;
    case TOKEN_OPERATOR:
        for (uint32 i = 0; i < operatorStrCount; i++) {
            if (avStringEquals(token.str, operatorStrs[i])) {

                return TOKEN_VALUE__OPERATOR + ((i + 1) << 4);
            }
        }
        avAssert(token.type == 0, "token invalid");
        return TOKEN_VALUE_TYPE_NONE;
    default:
        avAssert(0, "token invalid");
        return TOKEN_VALUE_TYPE_NONE;
    }
}

bool32 tokenIsType(Token token, TokenValueType type) {
    switch (type & 0xf) {
    case 0:
        switch (type) {
        case TOKEN_VALUE_TYPE_IDENTIFIER:
            return token.type == TOKEN_IDENTIFIER;
        case TOKEN_VALUE_TYPE_NUMBER_LITERAL:
            return token.type == TOKEN_NUMBER;
        case TOKEN_VALUE_TYPE_STRING_LITERAL:
            return token.type == TOKEN_STRING;
        default:
            avAssert(0, "invalid token type");
            return 0;
        }
        break;
    case PRE_PROCESSOR_VALUE_TYPE:
        if (token.type != TOKEN_PRE_PROCESSOR) {
            return false;
        }
        return avStringEquals(preprocessorStrs[(type - TOKEN_VALUE__PRE_PROCESSOR - 1) >> 4], token.str);
    case KEYWORD_VALUE_TYPE:
        if (token.type != TOKEN_KEYWORD) {
            return false;
        }
        return avStringEquals(keywordStrs[(type - TOKEN_VALUE__KEYWORD - 1) >> 4], token.str);
    case SPECIAL_VALUE_TYPE:
        if (token.type != TOKEN_SPECIAL) {
            return false;
        }
        return avStringEquals(specialStrs[(type - TOKEN_VALUE__SPECIAL - 1) >> 4], token.str);
    case OPERATOR_VALUE_TYPE:
        if (token.type != TOKEN_OPERATOR) {
            return false;
        }
        return avStringEquals(operatorStrs[(type - TOKEN_VALUE__OPERATOR - 1) >> 4], token.str);
    default:
        avAssert(0, "invalid token type");
        return 0;
    }

}