/*
 * door_time.cpp
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

#include "door.h"
#include "solar.h"

uint16_t resolve_door_time(
    const struct door_rule *rule,
    const struct solar_times *sol,
    bool is_open)
{
    int t = 0;

    switch (rule->ref) {

    case REF_NONE:
        /* disabled rule: caller must ignore */
        return 0;

    case REF_MIDNIGHT:
        t = rule->offset_minutes;
        break;

    case REF_SOLAR_STD:
        t = (is_open ? sol->sunrise_std : sol->sunset_std)
            + rule->offset_minutes;
        break;

    case REF_SOLAR_CIV:
        t = (is_open ? sol->sunrise_civ : sol->sunset_civ)
            + rule->offset_minutes;
        break;
    }

    while (t < 0)     t += 1440;
    while (t >= 1440) t -= 1440;

    return (uint16_t)t;
}
