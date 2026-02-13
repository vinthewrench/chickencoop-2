/*
 * config_events.cpp
 *
 * Project: Chicken Coop Controller
 * Purpose: Declarative schedule event storage
 *
 * Responsibilities:
 *  - Owns the persistent event table (g_cfg.events)
 *  - Provides read-only access to the sparse table
 *  - Performs ALL mutations of schedule intent
 *
 * Design rules:
 *  - This module is the single source of truth for schedule events
 *  - Any mutation MUST notify the scheduler
 *  - Read access MUST NOT have side effects
 *
 * Scheduler contract:
 *  - scheduler_touch() is called whenever the event table changes
 *  - This invalidates any cached reductions or next-event results
 *
 * Updated: 2026-01-08
 */

#include "config_events.h"
#include "config.h"
#include "scheduler.h"   /* schedule_touch() */

/* --------------------------------------------------------------------------
 * Accessor
 * --------------------------------------------------------------------------
 *
 * Returns:
 *  - Pointer to the full sparse event table (size MAX_EVENTS)
 *  - `count` receives the number of active events (refnum != 0)
 *
 * Notes:
 *  - The returned table contains unused slots
 *  - Caller MUST scan MAX_EVENTS and skip refnum == 0
 *  - This function is read-only and MUST NOT notify scheduler
 */
const Event *config_events_get(size_t *count)
{
    size_t n = 0;

    for (size_t i = 0; i < MAX_EVENTS; i++) {
        if (g_cfg.events[i].refnum != 0)
            n++;
    }

    if (count)
        *count = n;

    return g_cfg.events;
}

/* --------------------------------------------------------------------------
 * Add
 * --------------------------------------------------------------------------
 *
 * Inserts a new event into the first free slot.
 *
 * Behavior:
 *  - Assigns a stable, non-zero refnum (index + 1)
 *  - Fails if the table is full
 *
 * Scheduler impact:
 *  - Adds new schedule intent
 *  - MUST invalidate scheduler caches
 */
bool config_events_add(const Event *ev)
{
    if (!ev)
        return false;

    for (size_t i = 0; i < MAX_EVENTS; i++) {
        if (g_cfg.events[i].refnum == 0) {

            g_cfg.events[i] = *ev;
            g_cfg.events[i].refnum = (refnum_t)(i + 1);

            /* Schedule definition changed */
            schedule_touch();

            return true;
        }
    }

    return false; /* table full */
}

/* --------------------------------------------------------------------------
 * Update (by refnum)
 * --------------------------------------------------------------------------
 *
 * Replaces an existing event while preserving its identity.
 *
 * Behavior:
 *  - refnum selects the target event
 *  - refnum is preserved across update
 *
 * Scheduler impact:
 *  - Schedule intent has changed
 *  - MUST invalidate scheduler caches
 */
bool config_events_update_by_refnum(refnum_t ref, const Event *ev)
{
    if (!ev || ref == 0)
        return false;

    for (size_t i = 0; i < MAX_EVENTS; i++) {
        if (g_cfg.events[i].refnum == ref) {

            g_cfg.events[i] = *ev;
            g_cfg.events[i].refnum = ref;

            /* Schedule definition changed */
            schedule_touch();

            return true;
        }
    }

    return false;
}

/* --------------------------------------------------------------------------
 * Delete (by refnum)
 * --------------------------------------------------------------------------
 *
 * Removes an event from the table.
 *
 * Behavior:
 *  - Clears refnum to mark slot unused
 *  - Slot may be reused by future adds
 *
 * Scheduler impact:
 *  - Schedule intent has changed
 *  - MUST invalidate scheduler caches
 */
bool config_events_delete_by_refnum(refnum_t ref)
{
    if (ref == 0)
        return false;

    for (size_t i = 0; i < MAX_EVENTS; i++) {
        if (g_cfg.events[i].refnum == ref) {

            g_cfg.events[i].refnum = 0;

            /* Schedule definition changed */
            schedule_touch();

            return true;
        }
    }

    return false;
}

/* --------------------------------------------------------------------------
 * Clear
 * --------------------------------------------------------------------------
 *
 * Removes ALL events from the schedule.
 *
 * Behavior:
 *  - Marks all slots unused
 *
 * Scheduler impact:
 *  - Entire schedule definition replaced
 *  - MUST invalidate scheduler caches (once)
 */
void config_events_clear(void)
{
    for (size_t i = 0; i < MAX_EVENTS; i++)
        g_cfg.events[i].refnum = 0;

    /* Schedule definition changed */
    schedule_touch();
}
