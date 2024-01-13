#include "avCLexer.h"
#include "avCCompiler.h"
#include <AvUtils/memory/avAllocator.h>
#include <AvUtils/avLogging.h>
#include <AvUtils/string/avChar.h>
#include <string.h>

typedef struct Token Token;

typedef struct KeywordDetails {
    uint32 len;
    enum Keyword keyword;
}KeywordDetails;

typedef struct PunctuatorDetails {
    uint32 len;
    enum Punctuator punctuator;
}PunctuatorDetails;

typedef struct ConstantDetails {
    uint32 len;
    struct Constant constant;
}ConstantDetails;

typedef struct StringLiteralDetails {
    uint32 len;
    struct StringLiteral stringLiteral;
} StringLiteralDetails;

typedef struct IdentifierDetails {
    uint32 len;
    struct Identifier identifier;
} IdentifierDetails;

bool32 isKeyword(AvString str, KeywordDetails* details) {
#define KEYWORD(enum, str) AV_CSTRA(str),
    const AvString keywordStrings[] = {
        KEYWORD_LIST
    };
    const uint32 keywordCount = sizeof(keywordStrings) / sizeof(AvString);
#undef KEYWORD

    for (uint32 keywordIndex = 0; keywordIndex < keywordCount; keywordIndex++) {
        AvString keyword = keywordStrings[keywordIndex];
        if (str.len < keyword.len) {
            continue;
        }
        bool32 isThisKeyword = true;
        for (uint32 i = 0; i < keyword.len; i++) {
            if (i >= str.len) {
                isThisKeyword = false;
                break;
            }
            if (keyword.chrs[i] != str.chrs[i]) {
                isThisKeyword = false;
                break;
            }
        }
        if (!isThisKeyword) {
            continue;
        }
        if (str.len == keyword.len) {
            details->keyword = keywordIndex + 1;
            details->len = keyword.len;
            return true;
        }
        char nextChar = str.chrs[keyword.len];
        if (!avCharIsLetter(nextChar) && nextChar != '_') {
            details->keyword = keywordIndex + 1;
            details->len = keyword.len;
            return true;
        }
    }
    return false;
}

bool32 isPunctuator(AvString str, PunctuatorDetails* details) {
#define PUNCTUATOR(enum, str) AV_CSTRA(str),
    const AvString punctuatorStrings[] = {
        PUNCTUATOR_LIST
    };
    const uint32 punctuatorCount = sizeof(punctuatorStrings) / sizeof(AvString);
#undef PUNCTUATOR

    uint32 longestValidPunctuatorLength = 0;
    enum Punctuator longestPunctuator = PUNCTUATOR_NONE;

    for (uint32 punctuatorIndex = 0; punctuatorIndex < punctuatorCount; punctuatorIndex++) {
        AvString punctuator = punctuatorStrings[punctuatorIndex];
        if (str.len < punctuator.len) {
            continue;
        }
        bool32 isThisPunctuator = true;
        for (uint32 i = 0; i < punctuator.len; i++) {
            if (i >= str.len) {
                isThisPunctuator = false;
                break;
            }
            if (punctuator.chrs[i] != str.chrs[i]) {
                isThisPunctuator = false;
                break;
            }
        }
        if (!isThisPunctuator) {
            continue;
        }
        if (str.len == punctuator.len) {
            details->punctuator = punctuatorIndex + 1;
            details->len = punctuator.len;
            return true;
        }

        if (punctuator.len > longestValidPunctuatorLength) {
            longestPunctuator = punctuatorIndex + 1;
            longestValidPunctuatorLength = punctuator.len;
        }
    }
    if (longestPunctuator != PUNCTUATOR_NONE) {
        details->punctuator = longestPunctuator;
        details->len = longestValidPunctuatorLength;
        return true;
    }
    return false;
}

typedef struct IntegerConstantDetails {
    uint32 len;
    struct IntegerConstant integerConstant;
} IntegerConstantDetails;

typedef struct FloatingConstantDetails {
    uint32 len;
    struct FloatingConstant floatingConstant;
} FloatingConstantDetails;

typedef struct EnumerationConstantDetails {
    uint32 len;
    struct EnumerationConstant enumerationConstant;
} EnumerationConstantDetails;

typedef struct CharacterConstantDetails {
    uint32 len;
    struct CharacterConstant characterConstant;
} CharacterConstantDetails;

typedef struct DecimalConstantDetails {
    uint32 len;
    struct DecimalConstant decimalConstant;
} DecimalConstantDetails;

typedef struct OctalConstantDetails {
    uint32 len;
    struct OctalConstant octalConstant;
} OctalConstantDetails;

typedef struct HexadecimalConstantDetails {
    uint32 len;
    struct HexadecimalConstant hexadecimalConstant;
} HexadecimalConstantDetails;

typedef struct BinaryConstantDetails {
    uint32 len;
    struct BinaryConstant binaryConstant;
} BinaryConstantDetails;

bool32 isDecimalConstant(AvString str, DecimalConstantDetails* details) {
    if (str.chrs[0] == '0') {
        return false;
    }
    uint32 length = 0;
    while (length < str.len && avCharIsNumber(str.chrs[length])) {
        length++;
    }
    if (length == 0) {
        return false;
    }
    AvString decimalNumber = AV_STR(str.chrs, length);
    memcpy(&details->decimalConstant.str, &decimalNumber, sizeof(AvString));
    details->len = length;
    return true;
}

bool32 isOctalConstant(AvString str, OctalConstantDetails* details) {
    if (str.chrs[0] != '0') {
        return false;
    }
    uint32 length = 1;
    while (length < str.len && avCharIsWithinRange(str.chrs[length], '0', '7')) {
        length++;
    }
    AvString octalNumber = AV_STR(str.chrs, length);
    memcpy(&details->octalConstant.str, &octalNumber, sizeof(AvString));
    details->len = length;
    return true;
}


bool32 isHexadecimalConstant(AvString str, HexadecimalConstantDetails* details) {
    if (str.chrs[0] != '0') {
        return false;
    }
    if (str.len <= 2) {
        return false;
    }
    if (str.chrs[1] != 'x') {
        return false;
    }
    uint32 length = 2;
    while (length < str.len && avCharIsHexNumber(str.chrs[length])) {
        length++;
    }
    AvString hexNumber = AV_STR(str.chrs, length);
    memcpy(&details->hexadecimalConstant.str, &hexNumber, sizeof(AvString));
    details->len = length;
    return true;
}

bool32 isBinaryConstant(AvString str, BinaryConstantDetails* details) {
    if (str.chrs[0] != '0') {
        return false;
    }
    if (str.len <= 2) {
        return false;
    }
    if (str.chrs[1] != 'b') {
        return false;
    }
    uint32 length = 2;
    while (length < str.len && avCharIsWithinRange(str.chrs[length], '0', '1')) {
        length++;
    }
    AvString binaryNumber = AV_STR(str.chrs, length);
    memcpy(&details->binaryConstant.str, &binaryNumber, sizeof(AvString));
    details->len = length;
    return true;
}


bool32 isIntegerConstant(AvString str, IntegerConstantDetails* details) {
    DecimalConstantDetails decimalConstantDetails = { 0 };
    if (isDecimalConstant(str, &decimalConstantDetails)) {
        details->integerConstant.type = INTEGER_CONSTANT_TYPE_DECIMAL;
        memcpy(&details->integerConstant.decimalConstant, &decimalConstantDetails.decimalConstant, sizeof(struct DecimalConstant));
        details->len = decimalConstantDetails.len;
        goto isIntegerConstant_parseIntegerSuffix;
    }

    OctalConstantDetails octalConstantDetails = { 0 };
    if (isOctalConstant(str, &octalConstantDetails)) {
        details->integerConstant.type = INTEGER_CONSTANT_TYPE_OCTAL;
        memcpy(&details->integerConstant.octalConstant, &octalConstantDetails.octalConstant, sizeof(struct OctalConstant));
        details->len = octalConstantDetails.len;
        goto isIntegerConstant_parseIntegerSuffix;
    }

    HexadecimalConstantDetails hexadecimalConstantDetails = { 0 };
    if (isHexadecimalConstant(str, &hexadecimalConstantDetails)) {
        details->integerConstant.type = INTEGER_CONSTANT_TYPE_HEXADECIMAL;
        memcpy(&details->integerConstant.hexadecimalConstant, &hexadecimalConstantDetails.hexadecimalConstant, sizeof(struct HexadecimalConstant));
        details->len = hexadecimalConstantDetails.len;
        goto isIntegerConstant_parseIntegerSuffix;
    }

    BinaryConstantDetails binaryConstantDetails = { 0 };
    if (isBinaryConstant(str, &binaryConstantDetails)) {
        details->integerConstant.type = INTEGER_CONSTANT_TYPE_BINARY;
        memcpy(&details->integerConstant.binaryConstant, &binaryConstantDetails.binaryConstant, sizeof(struct BinaryConstant));
        details->len = binaryConstantDetails.len;
        goto isIntegerConstant_parseIntegerSuffix;
    }

    return false;
isIntegerConstant_parseIntegerSuffix:
    // integer suffix
    ;
    uint32 remainingLength = str.len - details->len;

#define INTEGER_SUFFIX(enum, str) AV_CSTRA(str),
    const AvString integerSuffixStrings[] = {
        INTEGER_SUFFIX_LIST
    };
    const uint32 integerSuffixCount = sizeof(integerSuffixStrings) / sizeof(AvString);
#undef INTEGER_SUFFIX
    enum IntegerSuffix suffix = INTEGER_SUFFIX_NONE;
    uint32 suffixLength = 0;

    while (remainingLength) {
        uint32 longestValidIntegerSuffixLength = 0;
        enum IntegerSuffix longestIntegerSuffix = INTEGER_SUFFIX_NONE;

        for (uint32 integerSuffixIndex = 0; integerSuffixIndex < integerSuffixCount; integerSuffixIndex++) {
            AvString integerSuffix = integerSuffixStrings[integerSuffixIndex];
            if (remainingLength < integerSuffix.len) {
                continue;
            }
            bool32 isThisIntegerSuffix = true;
            for (uint32 i = 0; i < integerSuffix.len; i++) {
                if (i >= remainingLength) {
                    isThisIntegerSuffix = false;
                    break;
                }
                if (integerSuffix.chrs[i] != str.chrs[i]) {
                    isThisIntegerSuffix = false;
                    break;
                }
            }
            if (!isThisIntegerSuffix) {
                continue;
            }
            if (remainingLength == integerSuffix.len) {
                suffix |= integerSuffixIndex + 1;
                suffixLength += integerSuffix.len;
                remainingLength -= integerSuffix.len;
                break;
            }

            if (integerSuffix.len > longestValidIntegerSuffixLength) {
                longestIntegerSuffix = integerSuffixIndex + 1;
                longestValidIntegerSuffixLength = integerSuffix.len;
                break;
            }
        }
        if (longestIntegerSuffix != INTEGER_SUFFIX_NONE) {
            suffix |= longestIntegerSuffix;
            suffixLength += longestValidIntegerSuffixLength;
            remainingLength -= longestValidIntegerSuffixLength;
        }
    }

    details->integerConstant.integerSuffix = suffix;
    details->len += suffixLength;
    return true;
}



bool32 isFloatingConstant(AvString str, FloatingConstantDetails* details) {
    if (str.chrs[0] != '.' && !avCharIsNumber(str.chrs[0])) {
        return false;
    }
    if (str.chrs[0] == '.') {


    } else if (avCharIsNumber(str.chrs[0])) {


    }
    return false;
}

bool32 isEnumerationConstant(AvString str, EnumerationConstantDetails* details) {

    return false;
}

bool32 isCharacterConstant(AvString str, CharacterConstantDetails* details) {

    return false;
}

bool32 isConstant(AvString str, ConstantDetails* details) {
    IntegerConstantDetails integerConstantDetails = { 0 };
    if (isIntegerConstant(str, &integerConstantDetails)) {
        details->constant.type = CONSTANT_TYPE_INTEGER;
        memcpy(&details->constant.integerConstant, &integerConstantDetails.integerConstant, sizeof(struct IntegerConstant));
        details->len = integerConstantDetails.len;
        return true;
    }
    FloatingConstantDetails floatingConstantDetails = { 0 };
    if (isFloatingConstant(str, &floatingConstantDetails)) {
        details->constant.type = CONSTANT_TYPE_FLOAT;
        memcpy(&details->constant.floatingConstant, &floatingConstantDetails.floatingConstant, sizeof(struct FloatingConstant));
        details->len = floatingConstantDetails.len;
        return true;
    }
    EnumerationConstantDetails enumerationConstantDetails = { 0 };
    if (isEnumerationConstant(str, &enumerationConstantDetails)) {
        details->constant.type = CONSTANT_TYPE_FLOAT;
        memcpy(&details->constant.enumerationConstant, &enumerationConstantDetails.enumerationConstant, sizeof(struct EnumerationConstant));
        details->len = enumerationConstantDetails.len;
        return true;
    }

    CharacterConstantDetails characterConstantDetails = { 0 };
    if (isCharacterConstant(str, &characterConstantDetails)) {
        details->constant.type = CONSTANT_TYPE_FLOAT;
        memcpy(&details->constant.characterConstant, &characterConstantDetails.characterConstant, sizeof(struct CharacterConstant));
        details->len = characterConstantDetails.len;
        return true;
    }

    return false;

}
bool32 isStringLiteral(AvString str, StringLiteralDetails* details) {

    return false;
}
bool32 isIdentifier(AvString str, IdentifierDetails* details) {

    return false;
}

TokenList tokenizeSource(AvString source) {
    AvAllocator scratch = AV_EMPTY;
    avAllocatorCreate(0, AV_ALLOCATOR_TYPE_DYNAMIC, &scratch);

    uint32 index = 0;
    while (index <= source.len) {
        AvString str = {
            .chrs = source.chrs + index,
            .len = source.len - index,
            .memory = nullptr
        };
        if (avCharIsWhiteSpace(str.chrs[0])) {
            continue;
        }

        Token* token = avAllocatorAllocate(sizeof(Token), &scratch);

        KeywordDetails keywordDetails = { 0 };
        if (isKeyword(str, &keywordDetails)) {
            token->type = TOKEN_TYPE_KEYWORD;
            token->keyword = keywordDetails.keyword;
            index += keywordDetails.len;
            continue;
        }

        PunctuatorDetails punctuatorDetails = { 0 };
        if (isPunctuator(str, &punctuatorDetails)) {
            token->type = TOKEN_TYPE_PUNCTUATOR;
            token->punctuator = punctuatorDetails.punctuator;
            index += punctuatorDetails.len;
            continue;
        }

        ConstantDetails constantDetails = { 0 };
        if (isConstant(str, &constantDetails)) {
            token->type = TOKEN_TYPE_CONSTANT;
            memcpy(&token->constant, &constantDetails.constant, sizeof(struct Constant));
            index += constantDetails.len;
            continue;
        }

        StringLiteralDetails stringLiteralDetails = { 0 };
        if (isStringLiteral(str, &stringLiteralDetails)) {
            token->type = TOKEN_TYPE_STRING_LITERAL;
            memcpy(&token->stringLiteral, &stringLiteralDetails.stringLiteral, sizeof(struct StringLiteral));
            index += stringLiteralDetails.len;
            continue;
        }

        IdentifierDetails identifierDetails = { 0 };
        if (isIdentifier(str, &identifierDetails)) {
            token->type = TOKEN_TYPE_IDENTIFIER;
            memcpy(&token->identifier, &identifierDetails.identifier, sizeof(struct Identifier));
            index += identifierDetails.len;
            continue;
        }

        // unexpected token
        avAssert(0, "unexpected token");
    }

    avAllocatorDestroy(&scratch);
    return (TokenList)AV_EMPTY;
}