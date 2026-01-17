/*
 * config_sw_avr.cpp
 *
 * Project: Chicken Coop Controller
 * Purpose: CONFIG slide switch (boot-time only)
 *
 * Notes:
 *  - Offline system
 *  - Deterministic behavior
 *  - No network dependencies
 *  - CONFIG is sampled once per boot and cached
 *
 * Hardware assumptions (LOCKED v2.5):
 *  - CONFIG slide switch is a static strap set BEFORE reset/power-up
 *  - Switch connects PC6 to GND when ON (active-low)
 *  - External pull-up resistor REQUIRED (~10 kΩ to VCC)
 *  - Internal pull-up enabled as secondary safety only
 *  - CONFIG is sampled once at boot and then ignored
 *  - CONFIG is NOT a wake source
 *
 * Updated: 2026-01-16
 */

// firmware/platform/config_sw_avr.cpp
#include "config_sw.h"
#include <avr/io.h>

/*
 * If wiring or polarity ever changes, this is the only file
 * that should need to be modified.
 */
#define CONFIG_SW_BIT PC6

/*
 * Read CONFIG strap once at boot.
 * Returns true if CONFIG MODE is active.
 */
static bool read_hw_state_once(void)
{
    /* PC6 as input */
    DDRC &= (uint8_t)~_BV(CONFIG_SW_BIT);

    /* internal pull-up (backup only) */
    PORTC |= _BV(CONFIG_SW_BIT);

    /* allow pin + RC + pull-up to settle */
    for (volatile uint8_t i = 0; i < 50; i++) {
        __asm__ __volatile__("nop");
    }

    /* active-low: LOW = CONFIG enabled */
    return (PINC & _BV(CONFIG_SW_BIT)) != 0;
}

/*
 * Public API:
 *   true  = CONFIG mode active
 *   false = normal operation
 */
bool config_sw_state(void)
{
    static int8_t cached = -1;

    if (cached < 0) {
        cached = read_hw_state_once() ? 1 : 0;
    }

    return cached != 0;
}
