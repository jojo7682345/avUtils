#ifndef __AV_TIME__
#define __AV_TIME__

#include "../avTypes.h"

#include "../avString.h"

typedef struct AvDateTime{
    uint8 second;
    uint8 minute;
    uint8 hour;
    uint8 day;
    uint8 month;
    uint16 year;
} AvDateTime;

typedef enum AvDateFormat {
    AV_DATE_FORMAT_SS_MM_HH_DD_MM_YYYY,
    AV_DATE_FORMAT_SS_MM_HH_DD_MM_YY,
    AV_DATE_FORMAT_YYYY_MM_DD_HH_MM_SS,
    AV_DATE_FORMAT_YY_MM_DD_HH_MM_SS,
    AV_DATE_FORMAT_SS_MM_HH,
    AV_DATE_FORMAT_DD_MM_YYYY,
    AV_DATE_FORMAT_DD_MM_YY,
    AV_DATE_FORMAT_YYYY_MM_DD,
    AV_DATE_FORMAT_YY_MM_DD,
}AvDateFormat;

void avTimeConvertToString(AvDateTime time, AvAllocatedString* result, AvDateFormat format);

#endif//__AV_TIME__