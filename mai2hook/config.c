#include <assert.h>
#include <stddef.h>

#include "board/config.h"

#include "hooklib/config.h"
#include "hooklib/dvd.h"
#include "hooklib/gfx.h"

#include "mai2hook/config.h"
#include "mai2hook/led.h"

#include "platform/config.h"

void mai2_dll_config_load(
        struct mai2_dll_config *cfg,
        const wchar_t *filename)
{
    assert(cfg != NULL);
    assert(filename != NULL);

    GetPrivateProfileStringW(
            L"mai2io",
            L"path",
            L"",
            cfg->path,
            _countof(cfg->path),
            filename);
}

void led_config_load(
        struct led_config *cfg,
        const wchar_t *filename)
{
    assert(cfg != NULL);
    assert(filename != NULL);

    cfg->enable = GetPrivateProfileIntW(L"led", L"enable", 1, filename);
}

void mai2_hook_config_load(
        struct mai2_hook_config *cfg,
        const wchar_t *filename)
{
    assert(cfg != NULL);
    assert(filename != NULL);

    platform_config_load(&cfg->platform, filename);
    aime_config_load(&cfg->aime, filename);
    dvd_config_load(&cfg->dvd, filename);
    io4_config_load(&cfg->io4, filename);
    gfx_config_load(&cfg->gfx, filename);
    mai2_dll_config_load(&cfg->dll, filename);
    led_config_load(&cfg->led, filename);
}
