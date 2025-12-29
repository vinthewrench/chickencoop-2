/*
 * uptime_host.cpp
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

#include "uptime.h"

#include <time.h>

static time_t g_start = 0;

void uptime_init(void)
{
    if (g_start == 0)
        g_start = time(0);
}

uint32_t uptime_seconds(void)
{
    if (g_start == 0)
        g_start = time(0);
    time_t now = time(0);
    return (uint32_t)(now - g_start);
}
