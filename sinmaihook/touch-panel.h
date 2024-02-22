#pragma once

#include <windows.h>
#include <stdint.h>
#include <stdbool.h>

#define TOUCH_PANEL_BAUD_RATE 9600

HRESULT touch_panel_hook_init();
void touch_panel_update_touch(int player, uint8_t *touch_report);

/*
area to code:
65, 66, 67, 68, 69, 70, 71, 72, 73, 74,
75, 76, 77, 78, 79, 80, 81, 82, 83, 84,
85, 86, 87, 88, 89, 90, 91, 92, 93, 94,
95, 96, 97, 98, 0
*/
enum TouchPanelArea
{
    A1,
    A2,
    A3,
    A4,
    A5,
    A6,
    A7,
    A8,
    B1,
    B2,
    B3,
    B4,
    B5,
    B6,
    B7,
    B8,
    C1,
    C2,
    D1,
    D2,
    D3,
    D4,
    D5,
    D6,
    D7,
    D8,
    E1,
    E2,
    E3,
    E4,
    E5,
    E6,
    E7,
    E8,
    Blank,
    End
};

struct touch_panel
{
    uint8_t player;
    char side;

    bool is_conditioning;

    uint8_t touch_report[7];

    uint8_t ratio[34];
    uint8_t sensitivity[34];
};

struct touch_panel_req
{
    char start_byte;
    char panel;
    char area;
    uint8_t cmd;
    uint8_t value;
    char end_byte;
};

enum touch_panel_cmd
{
    TP_CMD_RSET = 'E',
    TP_CMD_HALT = 'L',
    TP_CMD_STAT = 'A',
    TP_CMD_Ratio = 114,
    TP_CMD_Sensitivity = 107,
    TP_CMD_SensCheck = 116,
};
enum touch_panel_resp_size
{
    TP_REPORT_SIZE = 9,
    TP_PONG_SIZE = 6,
};
