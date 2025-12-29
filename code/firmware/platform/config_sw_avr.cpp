/*
 * config_sw_avr.cpp
 *
 * Project: Chicken Coop Controller
 * Purpose: Source file
 *
 * Notes:
 *  - Offline system
 *  - Deterministic behavior
 *  - No network dependencies
 *
 * Updated: 2025-12-29
 */

// firmware/platform/config_sw_avr.cpp
#include "config_sw.h"
#include <avr/io.h>

/*
 * Hardware assumption:
 * - CONFIG slide switch connects PB4 to GND when ON (active-low).
 * - Internal pull-up enabled.
 *
 * If your wiring/polarity changes, this is the only place to edit.
 */
#define CONFIG_SW_BIT PB4

static bool read_hw_state_once(void)
{
    // PB4 input
    DDRB  &= (uint8_t)~_BV(CONFIG_SW_BIT);
    // enable pull-up
    PORTB |= _BV(CONFIG_SW_BIT);

    // active-low
    return (PINB & _BV(CONFIG_SW_BIT)) ? false : true;
}

bool config_sw_state(void)
{
    static int8_t cached = -1;
    if (cached < 0) {
        cached = read_hw_state_once() ? 1 : 0;
    }
    return cached ? true : false;
}
