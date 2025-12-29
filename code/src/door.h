/*
 * door.h
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
#include <stdbool.h>

enum door_time_ref {
    REF_NONE = 0,
    REF_MIDNIGHT,
    REF_SOLAR_STD,
    REF_SOLAR_CIV
};
struct door_rule {
    enum door_time_ref ref;
    int16_t offset_minutes;
};

void door_open(void);
void door_close(void);
bool door_is_open(void);

struct solar_times;
uint16_t resolve_door_time(
    const struct door_rule *rule,
    const struct solar_times *sol,
    bool is_open);
