/*
 * door_lock_avr.cpp
 *
 * Project: Chicken Coop Controller
 * Purpose: Lock actuator control (AVR)
 *
 * Notes:
 *  - Hardware driver WITH safety timeout
 *  - Direction via INA / INB
 *  - Power gated via EN
 *  - NO current sensing (by design)
 *  - Time-limited pulse to protect lock solenoid
 *
 * Hardware (LOCKED, V3.0):
 *  - VNH7100BASTR H-bridge
 *  - LOCK_INA -> PF0
 *  - LOCK_INB -> PF1
 *  - LOCK_EN  -> PF4
 *
 * Updated: 2026-01-05
 */

#include "door_lock.h"

#include <avr/io.h>
#include <stdint.h>

/* --------------------------------------------------------------------------
 * Configuration (LOCKED)
 * -------------------------------------------------------------------------- */

#define LOCK_PULSE_MS   500u   /* Safe solenoid pulse duration */

/* --------------------------------------------------------------------------
 * Pin mapping
 * -------------------------------------------------------------------------- */

#define LOCK_INA_BIT   (1u << PF0)
#define LOCK_INB_BIT   (1u << PF1)
#define LOCK_EN_BIT    (1u << PF4)

/* --------------------------------------------------------------------------
 * Internal state
 * -------------------------------------------------------------------------- */

static uint8_t  g_lock_active = 0;
static uint32_t g_lock_t0_ms  = 0;

/* --------------------------------------------------------------------------
 * Helpers (masked writes only)
 * -------------------------------------------------------------------------- */

static inline void set_bits(uint8_t mask)
{
    PORTF |= mask;
}

static inline void clear_bits(uint8_t mask)
{
    PORTF &= (uint8_t)~mask;
}

static inline void lock_hw_stop(void)
{
    /* Disable power first, then neutralize direction */
    clear_bits(LOCK_EN_BIT);
    clear_bits(LOCK_INA_BIT | LOCK_INB_BIT);
}

/* --------------------------------------------------------------------------
 * Public API
 * -------------------------------------------------------------------------- */

void lock_init(void)
{
    /* Configure control pins as outputs */
    DDRF |= (LOCK_INA_BIT | LOCK_INB_BIT | LOCK_EN_BIT);

    /* Safe default */
    lock_hw_stop();

    g_lock_active = 0;
    g_lock_t0_ms  = 0;
}

void lock_engage(void)
{
    if (g_lock_active) {
        /* Ignore re-entry: do not extend pulse by accident */
        return;
    }

    /* INA=1, INB=0 */
    clear_bits(LOCK_INB_BIT);
    set_bits(LOCK_INA_BIT);

    /* EN=1 */
    set_bits(LOCK_EN_BIT);

    g_lock_t0_ms  = 0;   /* armed on first tick */
    g_lock_active = 1;
}

void lock_release(void)
{
    if (g_lock_active) {
        return;
    }

    /* INA=0, INB=1 */
    clear_bits(LOCK_INA_BIT);
    set_bits(LOCK_INB_BIT);

    /* EN=1 */
    set_bits(LOCK_EN_BIT);

    g_lock_t0_ms  = 0;   /* armed on first tick */
    g_lock_active = 1;
}

void lock_tick(uint32_t now_ms)
{
    if (!g_lock_active) {
        return;
    }

    /* Arm start time on first tick after command */
    if (g_lock_t0_ms == 0) {
        g_lock_t0_ms = now_ms;
        return;
    }

    if ((uint32_t)(now_ms - g_lock_t0_ms) >= LOCK_PULSE_MS) {
        lock_hw_stop();
        g_lock_active = 0;
        g_lock_t0_ms  = 0;
    }
}
