/*
 * door_device.cpp
 *
 * Project: Chicken Coop Controller
 * Purpose: Door device abstraction
 *
 * Notes:
 *  - Implements Device interface
 *  - Owns door + lock sequencing
 *  - No timing or sensing yet
 *  - OPEN always unlocks first
 *  - CLOSE unlocks, moves door, then engages lock
 *
 * Updated: 2026-01-05
 */

#include "device.h"
#include "door_hw.h"
#include "door_lock.h"

/*
 * Scheduler-visible state only.
 * This reflects requested intent, not physical truth.
 */
static dev_state_t door_state = DEV_STATE_UNKNOWN;

static dev_state_t door_get_state(void)
{
    return door_state;
}

static void door_set_state(dev_state_t state)
{
    if (state == door_state)
        return;

    door_state = state;

    switch (state) {

    case DEV_STATE_ON:
        /* OPEN */
        lock_release();              /* ALWAYS unlock first */
        door_hw_set_open_dir();
        door_hw_enable();
        break;

    case DEV_STATE_OFF:
        /* CLOSE */
        lock_release();              /* ALWAYS unlock first */
        door_hw_set_close_dir();
        door_hw_enable();

        /*
         * NOTE:
         * Lock is engaged immediately after close command.
         * Timing will be handled by door_control later.
         */
        lock_engage();
        break;

    default:
        /* UNKNOWN → safest action */
        door_hw_stop();
        break;
    }
}

static const char *door_state_string(dev_state_t state)
{
    switch (state) {
    case DEV_STATE_ON:  return "OPEN";
    case DEV_STATE_OFF: return "CLOSED";
    default:            return "UNKNOWN";
    }
}

Device door_device = {
    .name = "door",
    .get_state = door_get_state,
    .set_state = door_set_state,
    .state_string = door_state_string
};
