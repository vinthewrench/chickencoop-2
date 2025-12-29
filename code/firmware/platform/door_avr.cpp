/*
 * door_avr.cpp
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

// firmware/platform/door_avr.cpp
#include "door.h"

/*
 * Firmware door actuator stubs.
 * These are placeholders until real door hardware control
 * is implemented. They must be safe, side-effect free,
 * and satisfy the shared console API.
 */

static bool door_open_state = false;

void door_open(void)
{
    // TODO: replace with real actuator logic
    door_open_state = true;
}

void door_close(void)
{
    // TODO: replace with real actuator logic
    door_open_state = false;
}

bool door_is_open(void)
{
    return door_open_state;
}
