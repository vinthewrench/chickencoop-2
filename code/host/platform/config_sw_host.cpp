/*
 * config_sw_host.cpp
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

// host/platform/config_sw_host.cpp
#include "config_sw.h"

// Host build policy: always enter CONFIG console.
bool config_sw_state(void)
{
    return true;
}
