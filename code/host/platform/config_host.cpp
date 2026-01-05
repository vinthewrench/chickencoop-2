/*
 * config_host.cpp
 *
 * Project: Chicken Coop Controller
 * Purpose: Host-side configuration storage
 *
 * Notes:
 *  - Offline system
 *  - Deterministic behavior
 *  - No network dependencies
 *  - Uses identical validation rules as AVR
 *  - Config is self-describing (magic + version + checksum)
 *
 * Updated: 2026-01-05
 */

#include "config.h"

#include <stdio.h>
#include <stddef.h>

#define HOST_CFG_FILE "coop.cfg"

bool config_load(struct config *cfg)
{
    struct config tmp;

    FILE *f = fopen(HOST_CFG_FILE, "rb");
    if (!f) {
        /* No saved config */
        config_defaults(cfg);
        return false;
    }

    size_t n = fread(&tmp, sizeof(tmp), 1, f);
    fclose(f);

    if (n != 1) {
        /* Short or corrupt file */
        config_defaults(cfg);
        return false;
    }

    /* Validate identity */
    if (tmp.magic != CONFIG_MAGIC ||
        tmp.version != CONFIG_VERSION) {

        config_defaults(cfg);
        return false;
    }

    /* Validate checksum */
    uint16_t stored = tmp.checksum;
    uint16_t computed = config_fletcher16(
        &tmp,
        offsetof(struct config, checksum)
    );

    if (stored != computed) {
        config_defaults(cfg);
        return false;
    }

    *cfg = tmp;
    return true;
}

void config_save(const struct config *cfg)
{
    struct config tmp = *cfg;

    /* Enforce identity */
    tmp.magic   = CONFIG_MAGIC;
    tmp.version = CONFIG_VERSION;

    /* Compute checksum */
    tmp.checksum = 0;
    tmp.checksum = config_fletcher16(
        &tmp,
        offsetof(struct config, checksum)
    );

    FILE *f = fopen(HOST_CFG_FILE, "wb");
    if (!f)
        return;

    fwrite(&tmp, sizeof(tmp), 1, f);
    fclose(f);
}
