/*
 * config_events.cpp
 *
 * Project: Chicken Coop Controller
 * Purpose: Event configuration storage
 *
 * Updated: 2026-01-08
 */

#include "config_events.h"
#include "config.h"

/* --------------------------------------------------------------------------
 * Accessor
 * -------------------------------------------------------------------------- */

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
 * -------------------------------------------------------------------------- */

bool config_events_add(const Event *ev)
{
    if (!ev)
        return false;

    for (size_t i = 0; i < MAX_EVENTS; i++) {
        if (g_cfg.events[i].refnum == 0) {
            g_cfg.events[i] = *ev;
            g_cfg.events[i].refnum = (refnum_t)(i + 1); /* stable, non-zero */
            return true;
        }
    }

    return false; /* table full */
}

/* --------------------------------------------------------------------------
 * Update (by refnum)
 * -------------------------------------------------------------------------- */

bool config_events_update_by_refnum(refnum_t ref, const Event *ev)
{
    if (!ev || ref == 0)
        return false;

    for (size_t i = 0; i < MAX_EVENTS; i++) {
        if (g_cfg.events[i].refnum == ref) {
            g_cfg.events[i] = *ev;
            g_cfg.events[i].refnum = ref; /* preserve identity */
            return true;
        }
    }

    return false;
}

/* --------------------------------------------------------------------------
 * Delete (by refnum)
 * -------------------------------------------------------------------------- */

bool config_events_delete_by_refnum(refnum_t ref)
{
    if (ref == 0)
        return false;

    for (size_t i = 0; i < MAX_EVENTS; i++) {
        if (g_cfg.events[i].refnum == ref) {
            g_cfg.events[i].refnum = 0; /* mark unused */
            return true;
        }
    }

    return false;
}

/* --------------------------------------------------------------------------
 * Clear
 * -------------------------------------------------------------------------- */

void config_events_clear(void)
{
    for (size_t i = 0; i < MAX_EVENTS; i++)
        g_cfg.events[i].refnum = 0;
}
