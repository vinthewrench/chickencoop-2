/*
 * config_common.cpp
 *
 * Project: Chicken Coop Controller
 * Purpose: Shared configuration defaults
 *
 * Notes:
 *  - Used by both host and firmware
 *  - Must not include AVR- or platform-specific headers
 *  - All fields initialized explicitly
 *
 * Updated: 2025-12-29
 */

#include "config.h"
#include <string.h>

void config_defaults(struct config *cfg)
{
    /* Start from a known baseline */
    memset(cfg, 0, sizeof(*cfg));

    /* ---- Time / location defaults ---- */

    cfg->tz        = -6;   /* CST */
    cfg->honor_dst = 1;

    cfg->latitude  = 34.4653;
    cfg->longitude = -93.3628;

    /* ---- Door rules defaults ---- */
    /* These must match what your UI expects */

    /* Example (adjust to your actual struct):
     *
     * cfg->open_rule.type   = DOOR_SOLAR;
     * cfg->open_rule.offset = 0;
     *
     * cfg->close_rule.type   = DOOR_CIVIL;
     * cfg->close_rule.offset = 0;
     */

    /* ---- Any future fields MUST be initialized here ---- */
}
