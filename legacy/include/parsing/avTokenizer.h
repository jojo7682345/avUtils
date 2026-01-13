#ifndef __AV_TOKENIZER__
#define __AV_TOKENIZER__
#include "../avDefinitions.h"
C_SYMBOLS_START
#include <AvUtils/avString.h>

typedef struct AvToken {
    AvString text;
    uint32 line;
    uint64 character;
    uint64 type;
} AvToken;

typedef enum AvTokenRuleType {
    AV_TOKEN_RULE_TYPE_EXACT = 0b000,
    AV_TOKEN_RULE_TYPE_VALID_CHARS = 0b001,

    AV_TOKEN_RULE_TYPE_START_WITH_EXACT = 0b000,
    AV_TOKEN_RULE_TYPE_START_WITH_VALID_CHAR = 0b010,
} AvTokenRuleType;

typedef struct AvTokenRule{
    AvTokenRuleType type;
    AvString text;
    AvString startsWith;
    AvString endsWith;
    bool8 useEscape;
    uint64 tokenType;
}AvTokenRule; 

#define TOKEN_TYPE_IGNORE ((uint64)-1)

enum AvTokenizeResultCode{
    AV_TOKENIZE_RESULT_OK = 0,
    AV_TOKENIZE_RESULT_AMBIGOUS_TOKEN,
    AV_TOKENIZE_RESULT_NO_RULE_FOUND,
};

typedef struct AvTokenizeResult {
    enum AvTokenizeResultCode code;
    uint32 line;
    uint32 character;
} AvTokenizeResult; 

AvTokenizeResult avTokenizeString(AvString str, AvArrayRef tokens, const uint32 ruleCount, const AvTokenRule* const rules);

C_SYMBOLS_END
#endif//_AV_TOKENIZER__