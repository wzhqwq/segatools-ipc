#pragma once

#include <windows.h>
#include <stdint.h>

#include "hook/iobuf.h"

#include "sinmaihook/led.h"

#define LED_SYNC 224
#define LED_ESCAPE 208
#define LED_HOST_ID 1
#define MAX_LED_ENCODED_SIZE 80

HRESULT led_handle_write(struct led_board *board);

struct led_hdr
{
    uint8_t sync;
    uint8_t dst;
    uint8_t src;
    uint8_t length;
};

// The end of the packet is the sum of all the bytes except the sync and sum bytes
struct led_pkg
{
    struct led_hdr hdr;
    uint8_t payload[40];
};

// status about the packet
enum led_ack_status
{
    LED_ACK_DROP = 0,
    LED_ACK_OK = 1,
    LED_ACK_SUM_ERROR = 2,
    LED_ACK_PARITY_ERROR = 3,
    LED_ACK_FRAME_ERROR = 4,
    LED_ACK_OVERRUN_ERROR = 5,
    LED_ACK_OVERFLOW_ERROR = 6,
    LED_ACK_STATUS_INVALID = 255,
};
// status about cmd
enum led_ack_report
{
    LED_ACK_RETURN_OK = 1,
    LED_ACK_RETURN_BUSY = 2,
    LED_ACK_RETURN_UNKNOWN_CMD = 3,
    LED_ACK_RETURN_WRONG_PARAM = 4,
    LED_ACK_RETURN_INVALID = 255,
};
enum led_cmd_no
{
    LED_CMD_RESET = 16,
    LED_CMD_SET_TIMEOUT = 17,

    LED_CMD_SET_GS_SINGLE = 49,
    LED_CMD_SET_GS_MULTI = 50,
    LED_CMD_SET_GS_MULTI_FADE = 51,
    LED_CMD_SET_FET = 57,
    LED_CMD_DC_UPDATE = 59,
    LED_CMD_GS_UPDATE = 60,
    LED_CMD_SET_DC = 63,

    LED_CMD_SET_ROM = 123,
    LED_CMD_GET_ROM = 124,
    LED_CMD_ENABLE_RESPONSE = 125,
    LED_CMD_DISABLE_RESPONSE = 126,

    LED_CMD_SET_ALL = 130,
    LED_CMD_GET_BOARD_INFO = 240,
    LED_CMD_GET_BOARD_STATUS = 241,
    LED_CMD_GET_FIRMWARE_SUM = 242,
    LED_CMD_GET_PROTOCOL_VER = 243,
    LED_CMD_BOOT = 253,
};

struct led_ack
{
    uint8_t status;
    uint8_t cmd;
    uint8_t report;
    uint8_t body[40];
};
struct led_req
{
    uint8_t cmd;
    uint8_t body[40];
};

/*
Initialization process:
reset -> getBoardInfo and check -> getProtocolVer -> reset -> set, get and check epp_rom
-> setDc -> dcUpdate -> setGsMulti all to off -> setFet to off -> gsUpdate
-> getBoardStatus -> setTimeout to 10

Halt process:
reset -> to initialization process

Error process:
to initialization process

Firmware update process:
to halt process -> not implemented
*/

// cmd no = 16
// ResetCommand

// cmd no = 17
struct led_set_timeout_cmd_req
{
    uint8_t timeout;
};

// cmd no = 49
struct led_set_single_cmd_req
{
    // led position 0~11
    uint8_t index;
    struct led_color_24 color;
};

// cmd no = 50
struct led_set_multi_cmd_req
{
    struct led_range range;
    struct led_color_24 color;
    uint8_t speed;
};

// cmd no = 51
// SetLedGs8BitMultiFadeCommand is same as led_set_multi_cmd_req

// cmd no = 57
struct led_set_fet_cmd_req
{
    struct led_color_24 color;
};

// cmd no = 59
// SetDcUpdateCommand

// cmd no = 60
// SetLedGsUpdateCommand

// cmd no = 63
struct led_set_dc_cmd_req
{
    struct led_range range;
    struct led_color_24 color;
};

// cmd no = 123
struct led_set_rom_cmd_req
{
    uint8_t addr;
    uint8_t data;
};
// cmd no = 124
struct led_get_rom_cmd_req
{
    uint8_t addr;
};
struct led_get_rom_cmd_ack
{
    uint8_t data;
};

// cmd no = 125
// SetEnableResponseCommand
// cmd no = 126
// SetDisableResponseCommand

// cmd no = 130
struct led_set_all_cmd_req
{
    struct led_color_24 color[11];
};

// cmd no = 240
struct led_get_board_info_cmd_ack
{
    char board_no[8];
    uint8_t divider_ff;
    uint8_t firm_revision;
};

// cmd no = 241
struct led_get_board_status_cmd_ack
{
    uint8_t timeout_status;
    uint8_t timeout;
    uint8_t pwm_io;
    uint8_t fet_timeout;
};

// cmd no = 242
struct led_get_firm_sum_cmd_ack
{
    uint8_t firm_sum_upper;
    uint8_t firm_sum_lower;
};

// cmd no = 243
struct led_get_protocol_ver_cmd_ack
{
    uint8_t appli_mode;
    uint8_t major;
    uint8_t minor;
};

// cmd no = 253
// SetBootModeCommand
