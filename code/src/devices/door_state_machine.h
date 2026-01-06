/*
 * door_state_machine.h
 *
 * Project: Chicken Coop Controller
 * Purpose: Door motion state machine (internal)
 *
 * Notes:
 *  - Time-based motion (no sensors)
 *  - Abort-and-restart on new command
 *  - OPEN is always safe default
 *  - Lock coordinated on close completion
 */

#pragma once

#include <stdint.h>
#include "device.h"   /* for dev_state_t */

#ifdef __cplusplus
extern "C" {
#endif

/* Internal door motion states (private truth) */
typedef enum {
    DOOR_IDLE_UNKNOWN = 0,
    DOOR_IDLE_OPEN,
    DOOR_IDLE_CLOSED,
    DOOR_MOVING_OPEN,
    DOOR_MOVING_CLOSED
} door_motion_t;

/* Initialize state machine (boot-time) */
void door_sm_init(void);

/* Command intent (edge-triggered) */
void door_sm_request(dev_state_t state);

/* Periodic tick (must be called regularly) */
void door_sm_tick(uint32_t now_ms);

/* Query settled device-visible state */
dev_state_t door_sm_get_state(void);

/* Optional: query motion (for LED, debug) */
door_motion_t door_sm_get_motion(void);

#ifdef __cplusplus
}
#endif
