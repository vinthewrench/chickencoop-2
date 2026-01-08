/*
 * state_reducer.cpp
 *
 * Project: Chicken Coop Controller
 * Purpose: Reduce declarative schedule events into expected device state
 *
 * Notes:
 *  - Backward-looking reducer
 *  - No globals, no I/O, no execution
 *  - Operates on sparse event tables
 *  - Produces facts only
 *
 * Updated: 2026-01-08
 */

#include "state_reducer.h"
#include "resolve_when.h"

#include <string.h>

void state_reducer_run(const Event *events,
                       size_t table_size,
                       const struct solar_times *sol,
                       uint16_t now_minute,
                       struct reduced_state *out)
{
    if (!events || !out)
        return;

    /* Clear output */
    memset(out, 0, sizeof(*out));

    uint16_t best_minute[STATE_REDUCER_MAX_DEVICES];
    bool     have_minute[STATE_REDUCER_MAX_DEVICES];

    memset(have_minute, 0, sizeof(have_minute));

    for (size_t i = 0; i < table_size; i++) {
        const Event *ev = &events[i];

        /* Skip unused slots */
        if (ev->refnum == 0)
            continue;

        if (ev->device_id >= STATE_REDUCER_MAX_DEVICES)
            continue;

        uint16_t minute;
        if (!resolve_when(&ev->when, sol, &minute))
            continue;

        /* Ignore future intent */
        if (minute > now_minute)
            continue;

        /* Latest event <= now wins */
        if (!have_minute[ev->device_id] ||
            minute >= best_minute[ev->device_id]) {

            best_minute[ev->device_id] = minute;
            out->action[ev->device_id] = ev->action;
            out->has_action[ev->device_id] = true;
            have_minute[ev->device_id] = true;
        }
    }
}
