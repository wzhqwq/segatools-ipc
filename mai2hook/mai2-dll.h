#pragma once

#include <windows.h>

#include "mai2io/mai2io.h"

struct mai2_dll {
    uint16_t api_version;
    HRESULT (*init)(void);
    HRESULT (*poll)(void);
    void (*get_opbtns)(uint8_t *opbtn);
    void (*get_gamebtns)(uint16_t *player1, uint16_t *player2);
    void (*led_set_color)(uint8_t led, uint8_t r, uint8_t g, uint8_t b);
};

struct mai2_dll_config {
    wchar_t path[MAX_PATH];
};

extern struct mai2_dll mai2_dll;

HRESULT mai2_dll_init(const struct mai2_dll_config *cfg, HINSTANCE self);
