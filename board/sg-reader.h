#pragma once

#include <windows.h>

#include <stdbool.h>

#include "board/aime-dll.h"
#include "board/sg-led.h"

struct aime_config {
    struct aime_dll_config dll;
    bool enable;
};

HRESULT sg_reader_hook_init(
        const struct aime_config *cfg,
        unsigned int port_no,
        HINSTANCE self);

extern struct sg_led *sg_reader_custom_led;
