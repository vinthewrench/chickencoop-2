/*
 * solar.cpp
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

#include "solar.h"
#include <math.h>

/*
 * Platform-safe constants
 */
static constexpr double PI = 3.14159265358979323846;
static constexpr double DEG2RAD = PI / 180.0;
static constexpr double RAD2DEG = 180.0 / PI;

/*
 * Round solar event times to nearest minute.
 * Host: lround()
 * AVR: explicit arithmetic to avoid libm bloat and surprises
 */
static inline uint16_t round_minutes(double x)
{
#if defined(__AVR__)
    if (x >= 0.0)
        return (uint16_t)(x + 0.5);
    else
        return (uint16_t)(x - 0.5);
#else
    return (uint16_t)lround(x);
#endif
}

/*
 * Duration between two minute-of-day values
 */
static uint16_t duration(uint16_t start, uint16_t end)
{
    if (end >= start)
        return end - start;
    return (uint16_t)(24 * 60 - start + end);
}

/*
 * Day-of-year calculation
 */
static int doy(int y, int m, int d)
{
    static const int mdays[] =
        { 0,31,59,90,120,151,181,212,243,273,304,334 };

    bool leap = (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
    return mdays[m - 1] + d + ((leap && m > 2) ? 1 : 0);
}

/*
 * Core NOAA solar event calculation
 */
static bool calc_event(
    int day_of_year,
    double lat,
    double lon,
    int tz,
    double zenith,
    bool sunrise,
    double &minutes_out)
{
    double lngHour = lon / 15.0;

    double t = sunrise
        ? day_of_year + ((6.0  - lngHour) / 24.0)
        : day_of_year + ((18.0 - lngHour) / 24.0);

    double M = (0.9856 * t) - 3.289;

    double L = M
        + (1.916 * sin(M * DEG2RAD))
        + (0.020 * sin(2 * M * DEG2RAD))
        + 282.634;

    while (L < 0.0)    L += 360.0;
    while (L >= 360.0) L -= 360.0;

    double RA = RAD2DEG * atan(0.91764 * tan(L * DEG2RAD));
    while (RA < 0.0)    RA += 360.0;
    while (RA >= 360.0) RA -= 360.0;

    double Lq  = floor(L  / 90.0) * 90.0;
    double RAq = floor(RA / 90.0) * 90.0;
    RA = (RA + (Lq - RAq)) / 15.0;

    double sinDec = 0.39782 * sin(L * DEG2RAD);
    double cosDec = cos(asin(sinDec));

    double cosH =
        (cos(zenith * DEG2RAD) -
         sinDec * sin(lat * DEG2RAD)) /
        (cosDec * cos(lat * DEG2RAD));

    if (cosH > 1.0 || cosH < -1.0)
        return false;

    double H = sunrise
        ? 360.0 - RAD2DEG * acos(cosH)
        : RAD2DEG * acos(cosH);

    H /= 15.0;

    double T = H + RA - (0.06571 * t) - 6.622;
    double UT = T - lngHour;

    while (UT < 0.0)   UT += 24.0;
    while (UT >= 24.0) UT -= 24.0;

    minutes_out = (UT + tz) * 60.0;

    if (minutes_out < 0.0)      minutes_out += 1440.0;
    if (minutes_out >= 1440.0)  minutes_out -= 1440.0;

    return true;
}

/*
 * Public API
 */
bool solar_compute(
    uint16_t year,
    uint8_t  month,
    uint8_t  day,
    double   lat,
    double   lon,
    int8_t   tz,
    struct solar_times *out)
{
    if (!out)
        return false;

    int n = doy(year, month, day);

    double sr_std, ss_std, sr_civ, ss_civ;

    if (!calc_event(n, lat, lon, tz, 90.833, true,  sr_std)) return false;
    if (!calc_event(n, lat, lon, tz, 90.833, false, ss_std)) return false;
    if (!calc_event(n, lat, lon, tz, 96.0,   true,  sr_civ)) return false;
    if (!calc_event(n, lat, lon, tz, 96.0,   false, ss_civ)) return false;

    out->sunrise_std = round_minutes(sr_std);
    out->sunset_std  = round_minutes(ss_std);
    out->sunrise_civ = round_minutes(sr_civ);
    out->sunset_civ  = round_minutes(ss_civ);

    out->day_length =
        duration(out->sunrise_std, out->sunset_std);

    out->visible_length =
        duration(out->sunrise_civ, out->sunset_civ);

    return true;
}
