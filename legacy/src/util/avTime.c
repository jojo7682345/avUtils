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
    tm.tm_mon = time.month - 1;
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

    if (!now) {
        return (AvDateTime){0};  // fallback on failure
    }

    return (AvDateTime){
        .second = now->tm_sec,
        .minute = now->tm_min,
        .hour = now->tm_hour,
        .day = now->tm_mday,
        .month = now->tm_mon + 1,
        .year = now->tm_year+ 1900
    };
}

AvDateTime avTimeAdd(AvDateTime a, AvDateTime b) {
    // Initialize a 'struct tm' for 'a' date-time
    struct tm aTm = {
        .tm_year = a.year - 1900,
        .tm_mon = a.month - 1, // months are 0-based in struct tm
        .tm_mday = a.day,
        .tm_hour = a.hour,
        .tm_min = a.minute,
        .tm_sec = a.second,
    };

    // Add each component of 'b' to 'a'
    aTm.tm_year += b.year;   // this handles both years as offsets
    aTm.tm_mon += b.month - 1;
    aTm.tm_mday += b.day;
    aTm.tm_hour += b.hour;
    aTm.tm_min += b.minute;
    aTm.tm_sec += b.second;

    // Normalize the time using mktime
    time_t timeRes = mktime(&aTm);
    struct tm* rTm = localtime(&timeRes);

    // Return the result as AvDateTime
    return (AvDateTime){
        .year = rTm->tm_year + 1900,
        .month = rTm->tm_mon + 1, // Adjust back to 1-based month
        .day = rTm->tm_mday,
        .hour = rTm->tm_hour,
        .minute = rTm->tm_min,
        .second = rTm->tm_sec,
    };
}

AvDateTime avTimeSub(AvDateTime a, AvDateTime b) {
    // Initialize a 'struct tm' for 'a' date-time
    struct tm aTm = {
        .tm_year = a.year - 1900,
        .tm_mon = a.month - 1, // months are 0-based in struct tm
        .tm_mday = a.day,
        .tm_hour = a.hour,
        .tm_min = a.minute,
        .tm_sec = a.second,
    };

    // Subtract each component of 'b' from 'a'
    aTm.tm_year -= b.year;
    aTm.tm_mon -= b.month - 1;
    aTm.tm_mday -= b.day;
    aTm.tm_hour -= b.hour;
    aTm.tm_min -= b.minute;
    aTm.tm_sec -= b.second;

    // Normalize the time using mktime
    time_t timeRes = mktime(&aTm);
    struct tm* rTm = localtime(&timeRes);

    // Return the result as AvDateTime
    return (AvDateTime){
        .year = rTm->tm_year + 1900,
        .month = rTm->tm_mon + 1, // Adjust back to 1-based month
        .day = rTm->tm_mday,
        .hour = rTm->tm_hour,
        .minute = rTm->tm_min,
        .second = rTm->tm_sec,
    };
}

int64 avTimeConvertToNumber(AvDateTime time){
    struct tm aTm = {
        .tm_year = time.year - 1900,
        .tm_mon = time.month - 1,
        .tm_mday = time.day,
        .tm_hour = time.hour,
        .tm_min = time.minute,
        .tm_sec = time.second,
    };

    // Normalize the time using mktime
    time_t timeRes = mktime(&aTm);
    return (int64)timeRes;
}

AvDateTime avTimeConvertFromNumber(int64 number) {
    time_t tim = (time_t)number;
    struct tm* t = localtime(&tim);

    if (!t) {
        return (AvDateTime){0};  // fallback on failure
    }

    return (AvDateTime){
        .second = t->tm_sec,
        .minute = t->tm_min,
        .hour   = t->tm_hour,
        .day    = t->tm_mday,
        .month  = t->tm_mon + 1,      // 0–11 → 1–12
        .year   = t->tm_year + 1900
    };
}