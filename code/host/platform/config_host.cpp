/*
 * config_host.cpp
 *
 * Project: Chicken Coop Controller
 * Purpose: Host-side configuration source
 *
 * Notes:
 *  - Offline system
 *  - Deterministic behavior
 *  - No network dependencies
 *  - Uses shared defaults, then applies host overrides
 *
 * Updated: 2025-12-29
 */

#include "config.h"



void config_load(struct config *cfg)
{
    /* Start from shared defaults */
    config_defaults(cfg);

 /* fake for testing */
    cfg->open_rule  = { REF_SOLAR_STD, 0 };
    cfg->close_rule = { REF_SOLAR_CIV, 0 };
}

/* Host does not persist configuration */
void config_save(const struct config *)
{
    /* no-op */
}
