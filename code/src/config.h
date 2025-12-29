/*
 * config.h
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
#include "door.h"


struct config {
    double latitude;
    double longitude;
    int    tz;          // standard time offset
    bool   honor_dst;   // NEW: apply US DST rules
   struct door_rule open_rule;
    struct door_rule close_rule;
};

void config_load(struct config *cfg);
void config_save(const struct config *cfg);
void config_defaults(struct config *cfg);
