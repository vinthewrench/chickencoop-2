/*
 * rtc.h
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

 /*
  * Date convention:
  *   - year  = full year (e.g. 2025)
  *   - month = 1..12
  *   - day   = 1..31
  *
  * All date-handling code MUST follow this convention.
  */

#pragma once
#include <stdbool.h>
void rtc_get_time(int*,int*,int*,int*,int*,int*);

// Set RTC date/time (24-hour internally). Stubbed until RTC HW is wired.
void rtc_set_time(int y,int mo,int d,int h,int m,int s);

// True if RTC time is considered valid (set at least once).
bool rtc_time_is_set(void);
