#pragma once

#include <stdint.h>

/*
 * door_lock.h
 *
 * Project: Chicken Coop Controller
 * Purpose: Lock actuator control (abstract interface)
 *
 * Notes:
 *  - Lock is a non-self-limiting solenoid
 *  - Safety enforced via time-limited pulse
 *  - No current sensing (by design)
 *  - Host provides a stub implementation
 *
 * API rules:
 *  - lock_engage()  : lock the door (timed pulse)
 *  - lock_release() : unlock the door (timed pulse)
 *  - lock_tick()    : MUST be called periodically by firmware
 *
 * Updated: 2026-01-05
 */

/* Initialize hardware (idempotent) */
void lock_init(void);

/* Lock actuator control (fire-and-forget, timed internally) */
void lock_engage(void);
void lock_release(void);

/* Firmware loop hook (NO-OP on host) */
void lock_tick(uint32_t now_ms);
