/*
 * led_state_machine.cpp
 *
 * Project: Chicken Coop Controller
 * Purpose: Door status LED state machine
 *
 * Notes:
 *  - Non-blocking at the state-machine level
 *  - Software PWM carrier is driven by repeated door_led_tick() calls
 *  - Pulse envelope is rate-limited for smooth breathing
 *
 * Critical:
 *  - door_led_tick() advances an 8-bit PWM phase.
 *  - If called only 1 kHz, the PWM cycle is 1000/256 ≈ 3.9 Hz (visible flashing).
 *  - Therefore we MUST call door_led_tick() many times per millisecond.
 */

#include "led_state_machine.h"
#include "door_led.h"

#include <stdint.h>
#include <stdbool.h>

#define BLINK_PERIOD_MS   250u
#define PULSE_PERIOD_MS  1500u   /* full breathe cycle */

/*
 * Software PWM carrier rate.
 *
 * door_led_tick() increments an 8-bit phase, so:
 * PWM_CYCLE_HZ ≈ (PWM_TICKS_PER_MS * 1000) / 256
 *
 * PWM_TICKS_PER_MS = 32  -> ~125 Hz PWM cycle (usually flicker-free)
 * PWM_TICKS_PER_MS = 64  -> ~250 Hz PWM cycle (safer if you still see shimmer)
 */
#define PWM_TICKS_PER_MS  48u

/* --------------------------------------------------------------------------
 * Internal state
 * -------------------------------------------------------------------------- */

static led_mode_t  g_mode  = LED_OFF;
static led_color_t g_color = LED_GREEN;

static uint32_t g_blink_t0_ms     = 0;
static bool     g_led_on          = false;

static uint32_t g_pulse_last_ms   = 0;   /* NOTE: holds PWM ticks, not ms */
static uint8_t  g_pulse_step      = 0;
static uint32_t g_pwm_ticks       = 0;   /* increments once per door_led_tick() */

/* Remainder accumulator for pulse step distribution (ticks) */
static uint32_t g_pulse_err       = 0;

/* --------------------------------------------------------------------------
 * Perceptual breathing envelopes
 * -------------------------------------------------------------------------- */

/* GREEN ramp */
static const uint8_t pulse_lut_green[] = {
      0,  1,  2,  4,  7, 11, 16, 22,
     29, 37, 46, 56, 67, 79, 92,106,
    121,137,154,172,191,211,232,255,
    232,211,191,172,154,137,121,106,
     92, 79, 67, 56, 46, 37, 29, 22,
     16, 11,  7,  4,  2,  1
};

/* RED ramp (boosted) */
static const uint8_t pulse_lut_red[] = {
      0,  4,  7, 11, 16, 23, 31, 40,
     50, 61, 73, 86,100,115,131,148,
    166,185,205,225,245,252,255,255,
    252,245,225,205,185,166,148,131,
    115,100, 86, 73, 61, 50, 40, 31,
     23, 16, 11,  7,  4
};

#define PULSE_STEPS_GREEN ((uint8_t)sizeof(pulse_lut_green))
#define PULSE_STEPS_RED   ((uint8_t)sizeof(pulse_lut_red))

/* --------------------------------------------------------------------------
 * Helpers
 * -------------------------------------------------------------------------- */

static inline void led_apply(bool on, uint8_t duty)
{
    if (!on) {
        door_led_off();
        return;
    }

    if (g_color == LED_GREEN) {
        door_led_green_pwm(duty);
    } else {
        door_led_red_pwm(duty);
    }
}

/*
 * Drive the software PWM carrier by advancing door_led_tick() multiple
 * times per millisecond elapsed.
 *
 * This keeps the PWM cycle frequency high enough to avoid visible flashing.
 */
static void door_led_pwm_service(uint32_t now_ms)
{
    static uint32_t last_ms = 0;

    uint32_t elapsed = now_ms - last_ms;
    if (elapsed == 0)
        return;

    last_ms = now_ms;

    uint32_t ticks = elapsed * (uint32_t)PWM_TICKS_PER_MS;

    /*
     * Defensive clamp:
     * If the main loop stalls hard, avoid spending forever here.
     * 10 ms worth of catch-up is enough for visual continuity.
     */
    const uint32_t MAX_TICKS = 10u * (uint32_t)PWM_TICKS_PER_MS;
    if (ticks > MAX_TICKS)
        ticks = MAX_TICKS;

    while (ticks--) {
        door_led_tick();
        g_pwm_ticks++;   /* THIS WAS MISSING */
    }
}

/* --------------------------------------------------------------------------
 * Public API
 * -------------------------------------------------------------------------- */

void led_state_machine_init(void)
{
    g_mode           = LED_OFF;
    g_color          = LED_GREEN;

    g_blink_t0_ms    = 0;
    g_led_on         = false;

    g_pulse_last_ms  = 0;
    g_pulse_step     = 0;
    g_pwm_ticks      = 0;
    g_pulse_err      = 0;

    door_led_init();
    door_led_off();
}

void led_state_machine_set(led_mode_t mode, led_color_t color)
{
    g_mode           = mode;
    g_color          = color;

    g_blink_t0_ms    = 0;
    g_led_on         = false;

    g_pulse_last_ms  = 0;   /* re-latch epoch on next tick */
    g_pulse_step     = 0;
    g_pulse_err      = 0;

    if (mode == LED_OFF) {
        door_led_off();
    }
}

bool led_state_machine_is_on(void)
{
    return g_led_on;
}

void led_state_machine_tick(uint32_t now_ms)
{
    door_led_pwm_service(now_ms);

    switch (g_mode) {

    case LED_OFF:
        g_led_on = false;
        led_apply(false, 0);
        break;

    case LED_ON:
        g_led_on = true;
        led_apply(true, 255);
        break;

    case LED_BLINK:
        if (g_blink_t0_ms == 0)
            g_blink_t0_ms = now_ms;

        if ((uint32_t)(now_ms - g_blink_t0_ms) >= BLINK_PERIOD_MS) {
            g_led_on = !g_led_on;
            g_blink_t0_ms = now_ms;
        }

        led_apply(g_led_on, 255);
        break;

    case LED_PULSE: {
        const uint8_t *lut;
        uint8_t steps;

        if (g_color == LED_GREEN) {
            lut   = pulse_lut_green;
            steps = PULSE_STEPS_GREEN;
        } else {
            lut   = pulse_lut_red;
            steps = PULSE_STEPS_RED;
        }

        /* Pulse period expressed in PWM carrier ticks */
        const uint32_t period_ticks =
            (uint32_t)PULSE_PERIOD_MS * (uint32_t)PWM_TICKS_PER_MS;

        /* Base ticks per LUT step, plus remainder to distribute */
        const uint32_t base_step_ticks = period_ticks / (uint32_t)steps;
        const uint32_t rem_step_ticks  = period_ticks % (uint32_t)steps;

        /* Latch epoch once (epoch stored in g_pulse_last_ms as ticks) */
        if (g_pulse_last_ms == 0) {
            g_pulse_last_ms = g_pwm_ticks;
            g_pulse_step    = 0;
            g_pulse_err     = 0;
        }

        /* Advance envelope steps as needed (catch-up safe) */
        for (;;) {
            uint32_t elapsed_ticks = (uint32_t)(g_pwm_ticks - g_pulse_last_ms);

            uint32_t step_ticks = base_step_ticks;

            g_pulse_err += rem_step_ticks;
            if (g_pulse_err >= (uint32_t)steps) {
                g_pulse_err -= (uint32_t)steps;
                step_ticks += 1u;
            }

            if (elapsed_ticks < step_ticks)
                break;

            g_pulse_last_ms += step_ticks;

            g_pulse_step++;
            if (g_pulse_step >= steps)
                g_pulse_step = 0;
        }

        g_led_on = true;
        led_apply(true, lut[g_pulse_step]);
        break;
    }

    default:
        break;
    }
}
