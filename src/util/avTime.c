#include <AvUtils/util/avTime.h>
#include <AvUtils/avMath.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

/*
    AV_DATE_FORMAT_SS_MM_HH_DD_MM_YYYY,
    AV_DATE_FORMAT_SS_MM_HH_DD_MM_YY,
    AV_DATE_FORMAT_YYYY_MM_DD_HH_MM_SS,
    AV_DATE_FORMAT_YY_MM_DD_HH_MM_SS,
    AV_DATE_FORMAT_SS_MM_HH,
    AV_DATE_FORMAT_DD_MM_YYYY,
    AV_DATE_FORMAT_DD_MM_YY,
    AV_DATE_FORMAT_YYYY_MM_DD,
    AV_DATE_FORMAT_YY_MM_DD,
*/

void avTimeConvertToString(AvDateTime time, AvStringRef result, AvDateFormat format){

    const struct formatSize {
        const char* format;
        uint64 size;
    } formats[] = {
        {.format="%H:%M:%S %d-%m-%Y", .size=80},
        {.format="%H:%M:%S %d-%m-%y", .size=80},
        {.format="%Y-%m-%d %H:%M:%S", .size=80},
        {.format="%y-%m-%d %H:%M:%S", .size=80},
        {.format="%H:%M:%S", .size=8},
        {.format="%d-%m-%Y", .size=10},
        {.format="%d-%m-%y", .size=10},
        {.format="%Y-%m-%d", .size=10},
        {.format="%y-%m-%d", .size=10},
    };

    struct tm tm = { 0 };
    tm.tm_sec = time.second;
    tm.tm_min = time.minute;
    tm.tm_hour = time.hour;
    tm.tm_mday = time.day;
    tm.tm_mon = time.month;
    tm.tm_year = time.year - 1900;
    AvStringHeapMemory str;
    avStringMemoryHeapAllocate(formats[format].size, &str);
    strftime(str->data, formats[format].size, formats[format].format, &tm);
    avStringFromMemory(result, 0, AV_MIN(avCStringLength(str->data), formats[format].size), str);

}

bool32 avTimeIsEqual(AvDateTime time, AvDateTime compare){
    return memcmp(&time, &compare, sizeof(AvDateTime))==0;
}

bool32 avTimeIsBefore(AvDateTime time, AvDateTime compare){
    if(avTimeIsEqual(time, compare)){
        return false;
    }
    if(time.year < compare.year){
        return true;
    }
    if(time.month < compare.month){
        return true;
    }
    if(time.day < compare.day){
        return true;
    }
    if(time.hour < compare.hour){
        return true;
    }
    if(time.minute < compare.minute){
        return true;
    }
    if(time.second < compare.second){
        return true;
    }
    return false;
}
bool32 avTimeIsAfter(AvDateTime time, AvDateTime compare){
    if(avTimeIsEqual(time, compare)){
        return false;
    }
    return !avTimeIsBefore(time, compare);
}

AvDateTime avTimeNow(){
    time_t tim = time(NULL);
    struct tm* now = localtime(&tim);

    return (AvDateTime){
        .second = now->tm_sec,
        .minute = now->tm_min,
        .hour = now->tm_hour,
        .day = now->tm_mday,
        .month = now->tm_mon,
        .year = now->tm_year+ 1900
    };
}

AvDateTime avTimeAdd(AvDateTime a, AvDateTime b){
    struct tm aTm = {
        .tm_year = a.year - 1900,
        .tm_mon = a.month,
        .tm_mday = a.day,
        .tm_hour = a.hour,
        .tm_min = a.minute,
        .tm_sec = a.second,
    };
    struct tm bTm = {
        .tm_year = b.year - 1900,
        .tm_mon = b.month,
        .tm_mday = b.day,
        .tm_hour = b.hour,
        .tm_min = b.minute,
        .tm_sec = b.second,
    };

    time_t timeA = mktime(&aTm);
    time_t timeB = mktime(&bTm);

    time_t timeRes = timeA + timeB;
    struct tm* rTm = localtime(&timeRes);

    return (AvDateTime){
        .year = rTm->tm_year + 1900,
        .month = rTm->tm_mon,
        .day = rTm->tm_mday,
        .hour = rTm->tm_hour,
        .minute = rTm->tm_min,
        .second = rTm->tm_sec,
    };
}

AvDateTime avTimeSub(AvDateTime a, AvDateTime b){
    struct tm aTm = {
        .tm_year = a.year - 1900,
        .tm_mon = a.month,
        .tm_mday = a.day,
        .tm_hour = a.hour,
        .tm_min = a.minute,
        .tm_sec = a.second,
    };
    struct tm bTm = {
        .tm_year = b.year - 1900,
        .tm_mon = b.month,
        .tm_mday = b.day,
        .tm_hour = b.hour,
        .tm_min = b.minute,
        .tm_sec = b.second,
    };

    time_t timeA = mktime(&aTm);
    time_t timeB = mktime(&bTm);

    time_t timeRes = timeA - timeB;
    struct tm* rTm = localtime(&timeRes);

    return (AvDateTime){
        .year = rTm->tm_year + 1900,
        .month = rTm->tm_mon,
        .day = rTm->tm_mday,
        .hour = rTm->tm_hour,
        .minute = rTm->tm_min,
        .second = rTm->tm_sec,
    };
}