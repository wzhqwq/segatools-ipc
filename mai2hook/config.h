#pragma once

#include <stddef.h>

#include "board/config.h"

#include "hooklib/dvd.h"
#include "hooklib/gfx.h"

#include "mai2hook/mai2-dll.h"
#include "mai2hook/led.h"

#include "platform/config.h"

struct mai2_hook_config {
    struct platform_config platform;
    struct aime_config aime;
    struct dvd_config dvd;
    struct io4_config io4;
    struct gfx_config gfx;
    struct mai2_dll_config dll;
    struct led_config led;
};

void mai2_dll_config_load(
        struct mai2_dll_config *cfg,
        const wchar_t *filename);

void mai2_hook_config_load(
        struct mai2_hook_config *cfg,
        const wchar_t *filename);
