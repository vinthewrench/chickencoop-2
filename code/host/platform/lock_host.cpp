/*
 * lock_host.cpp
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

// host/platform/lock_host.cpp
#include "lock.h"
#include "console/mini_printf.h"

static bool lock_engaged = false;

void lock_engage(void)
{
    mini_printf("[HOST] lock_engage()\n");
    lock_engaged = true;
}

void lock_release(void)
{
    mini_printf("[HOST] lock_release()\n");
    lock_engaged = false;
}

bool lock_is_engaged(void)
{
    return lock_engaged;
}
