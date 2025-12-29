/*
 * main_host.cpp
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

#include "console/console.h"

#include "uptime.h"

int main(void){
    uptime_init();
    console_init();
    while(!console_should_exit()) console_poll();
    return 0;
}
