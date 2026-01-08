/*
 * solar.h
 *
 * Project: Chicken Coop Controller
 * Purpose: Solar time computation helpers
 *
 * Notes:
 *  - Offline system
 *  - Deterministic behavior
 *  - No network dependencies
 *
 * Design reality (important):
 *  - solar_compute() is pure math
 *  - compute_today_solar() is POLICY:
 *      * pulls date from console shadow / RTC
 *      * pulls lat/lon/tz/DST from config
 *      * therefore NOT pure and NOT reusable
 *
 * This file currently mixes:
 *  - low-level solar math
 *  - high-level application policy
 *
 * That is intentional (for now).
 *
 * Updated: 2026-01-08
 */

#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Solar times for a single calendar day.
 *
 * All values are minute-of-day (0..1439).
 */
struct solar_times {
    uint16_t sunrise_std;     /* Official sunrise */
    uint16_t sunset_std;      /* Official sunset */
    uint16_t sunrise_civ;     /* Civil dawn */
    uint16_t sunset_civ;      /* Civil dusk */
    uint16_t day_length;      /* sunrise_std → sunset_std */
    uint16_t visible_length;  /* sunrise_civ → sunset_civ */
};

/*
 * Pure solar computation.
 *
 * No globals.
 * No config.
 * No RTC.
 *
 * Caller must supply:
 *  - calendar date
 *  - latitude / longitude
 *  - timezone (already DST-adjusted if desired)
 */
bool solar_compute(
    uint16_t year,
    uint8_t  month,
    uint8_t  day,
    double   lat,
    double   lon,
    int8_t   tz,
    struct solar_times *out
);

#ifdef __cplusplus
}
#endif
