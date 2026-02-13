/*
 * door_hw.h
 *
 * Project: Chicken Coop Controller
 * Purpose: Door actuator hardware interface (VNH7100BASTR)
 *
 * Notes:
 *  - Pure hardware abstraction
 *  - Direction via INA / INB
 *  - Power gated via EN (digital enable, no PWM)
 *  - No timing, no state, no policy
 *  - Safe to call without explicit initialization
 *
 * LOCKED DESIGN
 *
 * Updated: 2026-01-05
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/* Set direction only (does not apply power) */
void door_hw_set_open_dir(void);    /* extend */
void door_hw_set_close_dir(void);   /* retract */

/* Power gate control */
void door_hw_enable(void);
void door_hw_disable(void);

/* Safe stop: EN=0, INA=0, INB=0 */
void door_hw_stop(void);

#ifdef __cplusplus
}
#endif
