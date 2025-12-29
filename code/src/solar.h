/*
 * solar.h
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

#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct solar_times {
    uint16_t sunrise_std;
    uint16_t sunset_std;
    uint16_t sunrise_civ;
    uint16_t sunset_civ;
    uint16_t day_length;
    uint16_t visible_length;
};

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
