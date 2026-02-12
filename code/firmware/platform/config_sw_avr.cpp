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
 * Hardware assumptions (LOCKED):
 *  - CONFIG slide switch is a static strap set BEFORE reset/power-up
 *
 * Electrical behavior (per schematic + verified):
 *  - Switch OPEN    → PC6 pulled HIGH → CONFIG MODE
 *  - Switch CLOSED  → PC6 tied to GND  → NORMAL MODE
 *
 * Firmware rules:
 *  - CONFIG is sampled once at boot and then ignored
 *  - CONFIG is NOT a wake source
 *
 * Updated: 2026-01-16
 */

#include "config_sw.h"
#include <avr/io.h>
#include "gpio_avr.h"

/*
 * Read CONFIG strap once at boot.
 *
 * Returns:
 *   true  = CONFIG MODE active
 *   false = normal operation
 *
 * ACTIVE-HIGH:
 *   PC6 HIGH -> CONFIG MODE
 *   PC6 LOW  -> normal mode
 */
 bool config_sw_state(void)
 {
     /* Ensure PC6 is input with pull-up enabled */
     DDRC  &= (uint8_t)~_BV(CONFIG_SW_BIT);
     PORTC |=  _BV(CONFIG_SW_BIT);

     /* ACTIVE-HIGH:
        HIGH = CONFIG
        LOW  = RUN
     */
     return (PINC & _BV(CONFIG_SW_BIT)) != 0;
 }
