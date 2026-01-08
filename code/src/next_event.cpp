/* ============================================================================
 * next_event.cpp
 *
 * Project: Chicken Coop Controller
 * Purpose: Determine the next scheduled event for today or tomorrow
 *
 * Notes:
 *  - Pure scheduling logic
 *  - No I/O
 *  - No globals
 *  - No device knowledge
 *  - Event table is SPARSE: skip entries with refnum == 0
 *
 * Updated: 2026-01-08
 * ========================================================================== */

#include "next_event.h"
#include "resolve_when.h"
#include "config_events.h"   /* MAX_EVENTS */

bool next_event_today(const Event *events,
                      size_t count,
                      const struct solar_times *sol,
                      uint16_t now_minute,
                      size_t *out_index,
                      uint16_t *out_minute,
                      bool *out_tomorrow)
{
    (void)count; /* informational only */

    if (!events || !out_index || !out_minute || !out_tomorrow)
        return false;

    bool found = false;
    uint16_t best_minute = 0;
    size_t best_index = 0;

    /* ------------------------------------------------------------
     * First pass: today (strictly after now)
     * ------------------------------------------------------------ */
    for (size_t i = 0; i < MAX_EVENTS; i++) {
        if (events[i].refnum == 0)
            continue;

        uint16_t minute;
        if (!resolve_when(&events[i].when, sol, &minute))
            continue;

        if (minute <= now_minute)
            continue;

        if (!found || minute < best_minute ||
            (minute == best_minute && i < best_index)) {
            found = true;
            best_minute = minute;
            best_index = i;
        }
    }

    if (found) {
        *out_index = best_index;
        *out_minute = best_minute;
        *out_tomorrow = false;
        return true;
    }

    /* ------------------------------------------------------------
     * Second pass: tomorrow (wrap to earliest valid event)
     * ------------------------------------------------------------ */
    found = false;
    best_minute = 0;
    best_index = 0;

    for (size_t i = 0; i < MAX_EVENTS; i++) {
        if (events[i].refnum == 0)
            continue;

        uint16_t minute;
        if (!resolve_when(&events[i].when, sol, &minute))
            continue;

        if (!found || minute < best_minute ||
            (minute == best_minute && i < best_index)) {
            found = true;
            best_minute = minute;
            best_index = i;
        }
    }

    if (!found)
        return false;

    *out_index = best_index;
    *out_minute = best_minute;
    *out_tomorrow = true;
    return true;
}
