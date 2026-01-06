/*
 * door_state_machine.cpp
 *
 * Project: Chicken Coop Controller
 * Purpose: Door motion state machine
 */

#include "door_state_machine.h"

#include "door_hw.h"
#include "door_lock.h"
#include "config.h"

/* --------------------------------------------------------------------------
 * Internal state
 * -------------------------------------------------------------------------- */

static door_motion_t g_motion = DOOR_IDLE_UNKNOWN;
static dev_state_t   g_settled_state = DEV_STATE_UNKNOWN;
static uint32_t      g_motion_t0_ms = 0;

/* --------------------------------------------------------------------------
 * Helpers
 * -------------------------------------------------------------------------- */

static void door_stop(void)
{
    door_hw_stop();
}

static void start_open(void)
{
    lock_release();
    door_hw_set_open_dir();
    door_hw_enable();

    g_motion = DOOR_MOVING_OPEN;
    g_motion_t0_ms = 0;   /* arm on first tick */
}

static void start_close(void)
{
    lock_release();
    door_hw_set_close_dir();
    door_hw_enable();

    g_motion = DOOR_MOVING_CLOSED;
    g_motion_t0_ms = 0;   /* arm on first tick */
}

/* --------------------------------------------------------------------------
 * Public API
 * -------------------------------------------------------------------------- */

void door_sm_init(void)
{
    door_stop();

    g_motion = DOOR_IDLE_UNKNOWN;
    g_settled_state = DEV_STATE_UNKNOWN;
    g_motion_t0_ms = 0;
}

void door_sm_request(dev_state_t state)
{
    if (state != DEV_STATE_ON && state != DEV_STATE_OFF)
        return;

    /* Abort any motion immediately */
    if (g_motion == DOOR_MOVING_OPEN || g_motion == DOOR_MOVING_CLOSED) {
        door_stop();
        g_motion = DOOR_IDLE_UNKNOWN;
        g_motion_t0_ms = 0;
    }

    if (state == DEV_STATE_ON) {
        start_open();
    } else {
        start_close();
    }
}

void door_sm_tick(uint32_t now_ms)
{
    if (g_motion != DOOR_MOVING_OPEN &&
        g_motion != DOOR_MOVING_CLOSED)
        return;

    /* Arm start time on first tick */
    if (g_motion_t0_ms == 0) {
        g_motion_t0_ms = now_ms;
        return;
    }

    if ((uint32_t)(now_ms - g_motion_t0_ms) < g_cfg.door_travel_ms)
        return;

    /* Motion complete */
    door_stop();

    if (g_motion == DOOR_MOVING_OPEN) {
        g_motion = DOOR_IDLE_OPEN;
        g_settled_state = DEV_STATE_ON;
    } else {
        g_motion = DOOR_IDLE_CLOSED;
        g_settled_state = DEV_STATE_OFF;
        lock_engage();
    }

    g_motion_t0_ms = 0;
}

dev_state_t door_sm_get_state(void)
{
    return g_settled_state;
}

door_motion_t door_sm_get_motion(void)
{
    return g_motion;
}
