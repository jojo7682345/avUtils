#include <AvUtils/parsing/avTokenizer.h>
#include <AvUtils/util/avBitfield.h>
#include <AvUtils/logging/avAssert.h>
#include <AvUtils/avMath.h>
#include <AvUtils/avMemory.h>
#include <AvUtils/string/avChar.h>
#include <string.h>


typedef enum RuleState{
    RULE_STATE_INVALID = 0,
    RULE_STATE_PARTIAL,
    RULE_STATE_COMPLETE,
    RULE_STATE_OVERRUN,
} RuleState;

RuleState checkString(AvString sequence, AvString str, uint64* index, uint64* validLength){
    if(sequence.len){
        for(; (*index) < str.len; (*index)++){
            if((*index) >= sequence.len){
                (*validLength) += sequence.len;
                return RULE_STATE_OVERRUN;
            }
            if(str.chrs[(*index)]!=sequence.chrs[(*index)]){
                return RULE_STATE_INVALID;
            }
        }
        if((*index) < sequence.len){
            return RULE_STATE_PARTIAL;
        }
        (*validLength) += sequence.len;
        return RULE_STATE_COMPLETE;
    }
    return RULE_STATE_OVERRUN;
}

RuleState testExactRule(AvTokenRule rule, AvString str){
    uint64 index = 0;
    uint64 validLength = 0;
    return checkString(rule.text, str, &index, &validLength);

}

RuleState checkEndsWith(AvString sequence, AvString str, bool32 useEscape){
    if(sequence.len){
        strOffset offset = avStringFindFirstOccuranceOf(str, sequence);
        if(offset == AV_STRING_NULL){
            return RULE_STATE_PARTIAL;
        }
        if(useEscape && offset>1){
            if(str.chrs[offset-1]=='\\'){
                return checkEndsWith(sequence, AV_STR(str.chrs+offset+1, str.len - offset -1), useEscape);
            }
        }
        if(offset == str.len-sequence.len){
            return RULE_STATE_COMPLETE;
        }
        if(offset < str.len-sequence.len){
            return RULE_STATE_OVERRUN;
        }
    }
    return RULE_STATE_PARTIAL;
}

RuleState testRangeRule(AvTokenRule rule, AvString str){
    uint64 index = 0;
    uint64 validLength = 0;
    RuleState result = RULE_STATE_COMPLETE;

    result = checkString(rule.startsWith, str, &index, &validLength);
    if(result == RULE_STATE_COMPLETE){
        return RULE_STATE_PARTIAL;
    }
    if(result!=RULE_STATE_OVERRUN){
        return result;
    }
    result = RULE_STATE_PARTIAL;
    uint64 varLength = 0;
    bool32 escaped = false;
    for(; index < str.len; index++){

        result = checkEndsWith(rule.endsWith, AV_STR(str.chrs + validLength, str.len - validLength), rule.useEscape);
        if(result!=RULE_STATE_PARTIAL){
            return result;
        }

        char chr = str.chrs[index];
        bool32 validChar = rule.text.len==0;
        if(avCharIsNewline(chr)){
            goto invalidChar;
        }

        if(rule.useEscape && !escaped){
            if(chr=='\\'){
                escaped = true;
                validChar = true;
                goto invalidChar;
            }
        }
        
        
        for(uint64 i = 0; i < rule.text.len; i++){
            char c = rule.text.chrs[i];
            
            if(c==chr){
                validChar = true;
                break;
            }
        }
        if(escaped){
            validChar = true;
            escaped = false;
        }

        invalidChar:
        if(!validChar){
            return varLength>0?RULE_STATE_OVERRUN:RULE_STATE_INVALID;
        }
        varLength++;
    }
    return RULE_STATE_PARTIAL;
}

RuleState testRule(AvTokenRule rule, AvString str){
    switch(rule.type){
        case AV_TOKEN_RULE_TYPE_EXACT:
            return testExactRule(rule, str);
        case AV_TOKEN_RULE_TYPE_VALID_CHARS:
            return testRangeRule(rule, str);
        default:
            avAssert(false, "not implemented yet");
            return RULE_STATE_INVALID;
    }
}

struct RuleCompletionInfo {
    uint32 rule;
    uint64 end;
};

void avTokenFree(AvToken token){
    avStringFree(&token.text);
}

void avTokenArrayFree(void* token, uint64 size){
    avAssert(sizeof(AvToken) == size, "must be token");
    avTokenFree(*(AvToken*)token);
}


AvTokenizeResult avTokenizeString(AvString str, AvArrayRef tokens, const uint32 ruleCount, const AvTokenRule* const rules){
    avAssert(tokens != nullptr, "tokens must be a valid reference");
    avAssert(rules != nullptr, "rules must be a valid pointer");
    avAssert(ruleCount != 0, "at least one rule must be specified");
    avAssert(str.len != 0, "input string cannot be empty");

    enum AvTokenizeResultCode result = AV_TOKENIZE_RESULT_OK;

    AvDynamicArray tmpTokens = AV_EMPTY;
    avDynamicArrayCreate(0, sizeof(AvToken), &tmpTokens);

    uint32 line = 1;
    uint32 character = 0;

    uint32 tokenStart = -1;

    RuleState* ruleStates = avCallocate(ruleCount, sizeof(RuleState), "allocating rule states");
    struct RuleCompletionInfo* validTokens = avCallocate(ruleCount, sizeof(struct RuleCompletionInfo), "allocating rule states");
    uint32 validTokenCount = 0;

    for(uint32 i = 0; i < ruleCount; i++){
        ruleStates[i] = RULE_STATE_PARTIAL;
    }

    for(uint32 index = 0; index <= str.len; index++){
        char c = '\0';
        if(index <= str.len){
            c = str.chrs[index];
        }
        character++;
        if(c == '\n'){
            line++;
            character = 1;
        }

        if(tokenStart==-1){
            tokenStart = index;
        }

        AvString tokenStr = {
            .chrs = str.chrs + tokenStart,
            .len = index - tokenStart + 1,
        };

        uint32 partialCount = 0;
        uint32 overrunCount = 0;
        uint32 completeCount = 0;
        
        for(uint32 i = 0; i < ruleCount; i++){
            if(ruleStates[i] == RULE_STATE_INVALID){
                continue;
            }

            RuleState res = testRule(rules[i], tokenStr);
            RuleState prevRes = ruleStates[i];
            ruleStates[i] = res;

            if(res == RULE_STATE_INVALID){
                continue;
            }

            if(res == RULE_STATE_PARTIAL){
                partialCount++;
            }
            if(res == RULE_STATE_OVERRUN){
                overrunCount++;
            }
            if(res == RULE_STATE_COMPLETE){
                completeCount++;
            }

            if((prevRes==RULE_STATE_COMPLETE || prevRes==RULE_STATE_PARTIAL) && res == RULE_STATE_OVERRUN){
                validTokens[validTokenCount++] = (struct RuleCompletionInfo){
                    .rule = i,
                    .end = index-1,
                };
            }
        }
        if(validTokenCount == 0){
            if(!(completeCount || partialCount || overrunCount)){
                if(avCharIsWhiteSpace(c)){
                    goto resetToken;
                }
                if(index == str.len){
                    result = AV_TOKENIZE_RESULT_OK;
                    goto tokenizingFailed;
                }
                result = AV_TOKENIZE_RESULT_NO_RULE_FOUND;
                goto tokenizingFailed;
            }
            continue;
        }
        if(completeCount == 0 && partialCount == 0){
            uint64 longestToken = -1;
            uint32 ruleCollision = 0;
            uint32 longestRule = -1;
            uint64 ruleType = -1;
            for(uint32 i = 0; i < validTokenCount; i++){
                struct RuleCompletionInfo info = validTokens[i];
                if(longestToken==-1){
                    longestToken = info.end;
                    longestRule = info.rule;
                    ruleCollision = 1;
                    ruleType = rules[info.rule].tokenType;
                    continue;
                }
                if(info.end == longestToken){
                    if(ruleType != rules[info.rule].tokenType){
                        ruleCollision++;
                    }
                    continue;
                }
                if(info.end > longestToken){
                    longestToken = info.end;
                    longestRule = info.rule;
                    ruleCollision = 1;
                    ruleType = rules[info.rule].tokenType;
                    continue;
                }
            }
            if(ruleCollision>1){
                result = AV_TOKENIZE_RESULT_AMBIGOUS_TOKEN;
                goto tokenizingFailed;
            }
            AvToken token = {
                .character = character,
                .line = line,
                .text = {
                    .chrs = tokenStr.chrs,
                    .len = tokenStr.len-1,
                    .memory = tokenStr.memory,
                },
                .type = rules[longestRule].tokenType
            };
            index = longestToken;
            if(token.type == TOKEN_TYPE_IGNORE){
                goto resetToken;
            }
            avDynamicArrayAdd(&token, tmpTokens);
        resetToken:
            tokenStart = -1;
            validTokenCount = 0;
            for(uint32 i = 0; i < ruleCount; i++){
                ruleStates[i] = RULE_STATE_PARTIAL;
            }
        }
    }

tokenizingFailed:
    if(result == AV_TOKENIZE_RESULT_OK){
        avArrayAllocateWithFreeCallback(avDynamicArrayGetSize(tmpTokens), sizeof(AvToken), tokens, false, avTokenArrayFree, nullptr);
        for(uint32 i = 0; i < avDynamicArrayGetSize(tmpTokens); i++){
            AvToken token = {0};
            avDynamicArrayRead(&token, i, tmpTokens);
            AvToken newToken = {
                .line = token.line,
                .character = token.character,
                .type = token.type,
            };
            avStringClone(&newToken.text, token.text);
            avArrayWrite(&newToken, i, tokens);
        }
    }


    avDynamicArrayDestroy(tmpTokens);
    avFree(validTokens);
    avFree(ruleStates);
    return (AvTokenizeResult) {
        .code = result,
        .line = line,
        .character = character,
    };
}