/*
 * lock_avr.cpp
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

// firmware/platform/lock_avr.cpp
#include "lock.h"

static bool lock_engaged = false;

void lock_engage(void)
{
    // TODO: replace with real hardware logic
    lock_engaged = true;
}

void lock_release(void)
{
    // TODO: replace with real hardware logic
    lock_engaged = false;
}

bool lock_is_engaged(void)
{
    return lock_engaged;
}
