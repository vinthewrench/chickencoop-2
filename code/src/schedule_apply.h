#pragma once

#include "state_reducer.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Apply the scheduler-derived intent to devices.
 *
 * Responsibilities:
 *  - Compare desired vs current device state
 *  - Issue device commands only when a change is required
 *
 * Notes:
 *  - No timing logic
 *  - No scheduling logic
 *  - Safe to call once per minute
 */
void schedule_apply(const struct reduced_state *rs);

#ifdef __cplusplus
}
#endif
