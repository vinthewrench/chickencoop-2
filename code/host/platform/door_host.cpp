/*
 * door_host.cpp
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

// host/platform/door_host.cpp
#include "door.h"
#include "console/mini_printf.h"

static bool door_open_state = false;

void door_open(void)
{
    mini_printf("[HOST] door_open()\n");
    door_open_state = true;
}

void door_close(void)
{
    mini_printf("[HOST] door_close()\n");
    door_open_state = false;
}

bool door_is_open(void)
{
    return door_open_state;
}
