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

void avTimeConvertToString(AvDateTime time, AvAllocatedString* result, AvDateFormat format){

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
    AvPersistentStringMemory str;
    avStringMemoryCreatePersistent(&str);
    avStringMemoryAllocate(formats[format].size, str);
    strftime(str->data, formats[format].size, formats[format].format, &tm);
    avStringMemoryCreateString(0, AV_MIN(avStringLength(str->data),formats[format].size), result, str);

}