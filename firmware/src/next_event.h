/* ============================================================================
 * next_event.h
 *
 * Project: Chicken Coop Controller
 * Purpose: Determine the next scheduled event for today (or tomorrow wrap)
 *
 * Notes:
 *  - Pure scheduling logic
 *  - No I/O, no globals, no device knowledge
 *  - Input event table is SPARSE: slots are valid iff refnum != 0
 *
 * Updated: 2026-01-08
 * ========================================================================== */

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
 *  events        - pointer to a sparse event table
 *  count         - number of used slots (informational only, may be 0)
 *  sol           - solar times for today (may be NULL)
 *  now_minute    - current minute-of-day (0..1439)
 *  out_index     - slot index into events[] for the next event (0..MAX_EVENTS-1)
 *  out_minute    - resolved minute-of-day of next event
 *  out_tomorrow  - true if wrapped to tomorrow
 *
 * Returns:
 *  true  if an event was found
 *  false if no usable events exist
 *
 * IMPORTANT:
 *  - Implementations must scan the full table width (MAX_EVENTS) and
 *    skip unused slots (refnum == 0). Do NOT use `count` as a loop bound.
 */
bool next_event_today(const Event *events,
                      size_t count,
                      const struct solar_times *sol,
                      uint16_t now_minute,
                      size_t *out_index,
                      uint16_t *out_minute,
                      bool *out_tomorrow);
