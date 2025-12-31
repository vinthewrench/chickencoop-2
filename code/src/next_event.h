/*
 * next_event.h
 *
 * Project: Chicken Coop Controller
 * Purpose: Determine the next scheduled event for today
 *
 * Notes:
 *  - Pure scheduling logic
 *  - No I/O
 *  - No globals
 *  - No device knowledge
 *
 * Updated: 2025-12-31
 */

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "events.h"
#include "solar.h"

/*
 * Find the next event occurring after now_minute.
 *
 * Parameters:
 *  events        - event table
 *  count         - number of events
 *  sol           - solar times for today (may be NULL)
 *  now_minute    - current minute-of-day (0..1439)
 *  out_index     - index into events[] of next event
 *  out_minute    - resolved minute-of-day of next event
 *  out_tomorrow  - set true if wrapped to tomorrow
 *
 * Returns:
 *  true  if an event was found
 *  false if no events exist
 */
bool next_event_today(const Event *events,
                      size_t count,
                      const struct solar_times *sol,
                      uint16_t now_minute,
                      size_t *out_index,
                      uint16_t *out_minute,
                      bool *out_tomorrow);
