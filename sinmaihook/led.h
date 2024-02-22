#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "hooklib/uart.h"

#define LED_BOARD_NO "15070-02"
#define LED_FIRM_VER 144
#define LED_NODE_ID 17
#define LED_FIRM_SUM 44535

struct led_color_32
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
};

struct led_color_24
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

struct led_range
{
    uint8_t start;
    uint8_t end;
    uint8_t skip;
};

struct led_epp_rom
{
    uint8_t Enable;
    uint8_t Fet0;
    uint8_t Fet1;
    uint8_t Fet2;
    uint8_t DcRed;
    uint8_t DcGreen;
    uint8_t DcBlue;
    uint8_t Out15;
    uint8_t End;
};

struct led_board
{
    uint8_t player;
    struct led_color_24 colors[11];
    struct led_epp_rom eep_rom;
    bool response_enabled;
    uint8_t timeout;
    struct uart *uart;
};
