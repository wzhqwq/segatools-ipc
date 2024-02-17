#pragma once

#include <windows.h>

#include <stdbool.h>

struct led_config {
    bool enable;
};

HRESULT mai2_led_hook_init(
        const struct led_config *cfg,
        unsigned int port_no,
        HINSTANCE self);

