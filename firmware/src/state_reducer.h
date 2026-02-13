/*
 * state_reducer.h
 *
 * Project: Chicken Coop Controller
 * Purpose: Reduce declarative schedule events into expected device state
 *
 * Rules:
 *  - Pure reducer (no side effects)
 *  - No I/O
 *  - No globals
 *  - No execution or replay
 *  - Deterministic and backward-looking
 *
 * Notes:
 *  - Safe to call at boot, after RTC set, or after crash
 *  - Latest event <= now_minute wins per device
 *  - Operates on a SPARSE event table
 *
 * Updated: 2026-01-08
 */

#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "events.h"
#include "solar.h"

/* Must cover all possible device IDs */
#define STATE_REDUCER_MAX_DEVICES 8

/*
 * Reduced, device-centric view of scheduler intent.
 * One slot per device ID.
 */
struct reduced_state {
    bool   has_action[STATE_REDUCER_MAX_DEVICES];
    Action action[STATE_REDUCER_MAX_DEVICES];
};

/*
 * Reduce events into expected device state at `now_minute`.
 *
 * Parameters:
 *  events      - sparse declarative event table
 *  table_size  - total size of the table (MAX_EVENTS)
 *  sol         - resolved solar times for today (may be NULL)
 *  now_minute  - current minute-of-day (0..1439)
 *  out         - reduced device state (cleared by reducer)
 */
void state_reducer_run(const Event *events,
                       size_t table_size,
                       const struct solar_times *sol,
                       uint16_t now_minute,
                       struct reduced_state *out);
