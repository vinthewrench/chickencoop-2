/*
 * rtc_host.cpp
 *
 * Project: Chicken Coop Controller
 * Purpose: Source file
 *
 * Notes:
 *  - Offline system
 *  - Deterministic behavior
 *  - No network dependencies
 *
 * Updated: 2025-12-29
 */

#include "rtc.h"
#include <time.h>
void rtc_get_time(int*y,int*mo,int*d,int*h,int*m,int*s){
    time_t t=time(0); struct tm *tm=localtime(&t);
    *y=tm->tm_year+1900; *mo=tm->tm_mon+1; *d=tm->tm_mday;
    *h=tm->tm_hour; *m=tm->tm_min; *s=tm->tm_sec;
}

void rtc_set_time(int,int,int,int,int,int)
{
    // Host does not set system time.
}

bool rtc_time_is_set(void)
{
    return true;
}
