#include <windows.h>

#include <limits.h>
#include <stdint.h>

#include "mai2io/mai2io.h"
#include "mai2io/config.h"

#include "util/dprintf.h"

static uint8_t mai2_opbtn;
static uint16_t mai2_player1_btn;
static uint16_t mai2_player2_btn;
static struct mai2_io_config mai2_io_cfg;

uint16_t mai2_io_get_api_version(void)
{
    return 0x0100;
}

HRESULT mai2_io_init(void)
{
    mai2_io_config_load(&mai2_io_cfg, L".\\segatools.ini");

    return S_OK;
}

HRESULT mai2_io_poll(void)
{
    mai2_opbtn = 0;
    mai2_player1_btn = 0;
    mai2_player2_btn = 0;

    if (GetAsyncKeyState(mai2_io_cfg.vk_test)) {
        mai2_opbtn |= MAI2_IO_OPBTN_TEST;
    }

    if (GetAsyncKeyState(mai2_io_cfg.vk_service)) {
        mai2_opbtn |= MAI2_IO_OPBTN_SERVICE;
    }

    //Player 1
    if (GetAsyncKeyState(mai2_io_cfg.vk_1p_btn[0])) {
        mai2_player1_btn |= MAI2_IO_GAMEBTN_1;
    }

    if (GetAsyncKeyState(mai2_io_cfg.vk_1p_btn[1])) {
        mai2_player1_btn |= MAI2_IO_GAMEBTN_2;
    }

    if (GetAsyncKeyState(mai2_io_cfg.vk_1p_btn[2])) {
        mai2_player1_btn |= MAI2_IO_GAMEBTN_3;
    }

    if (GetAsyncKeyState(mai2_io_cfg.vk_1p_btn[3])) {
        mai2_player1_btn |= MAI2_IO_GAMEBTN_4;
    }

    if (GetAsyncKeyState(mai2_io_cfg.vk_1p_btn[4])) {
        mai2_player1_btn |= MAI2_IO_GAMEBTN_5;
    }

    if (GetAsyncKeyState(mai2_io_cfg.vk_1p_btn[5])) {
        mai2_player1_btn |= MAI2_IO_GAMEBTN_6;
    }

    if (GetAsyncKeyState(mai2_io_cfg.vk_1p_btn[6])) {
        mai2_player1_btn |= MAI2_IO_GAMEBTN_7;
    }

    if (GetAsyncKeyState(mai2_io_cfg.vk_1p_btn[7])) {
        mai2_player1_btn |= MAI2_IO_GAMEBTN_8;
    }

    if (GetAsyncKeyState(mai2_io_cfg.vk_1p_btn[8])) {
        mai2_player1_btn |= MAI2_IO_GAMEBTN_SELECT;
    }

    //Player 2
    if (GetAsyncKeyState(mai2_io_cfg.vk_2p_btn[0])) {
        mai2_player2_btn |= MAI2_IO_GAMEBTN_1;
    }

    if (GetAsyncKeyState(mai2_io_cfg.vk_2p_btn[1])) {
        mai2_player2_btn |= MAI2_IO_GAMEBTN_2;
    }

    if (GetAsyncKeyState(mai2_io_cfg.vk_2p_btn[2])) {
        mai2_player2_btn |= MAI2_IO_GAMEBTN_3;
    }

    if (GetAsyncKeyState(mai2_io_cfg.vk_2p_btn[3])) {
        mai2_player2_btn |= MAI2_IO_GAMEBTN_4;
    }

    if (GetAsyncKeyState(mai2_io_cfg.vk_2p_btn[4])) {
        mai2_player2_btn |= MAI2_IO_GAMEBTN_5;
    }

    if (GetAsyncKeyState(mai2_io_cfg.vk_2p_btn[5])) {
        mai2_player2_btn |= MAI2_IO_GAMEBTN_6;
    }

    if (GetAsyncKeyState(mai2_io_cfg.vk_2p_btn[6])) {
        mai2_player2_btn |= MAI2_IO_GAMEBTN_7;
    }

    if (GetAsyncKeyState(mai2_io_cfg.vk_2p_btn[7])) {
        mai2_player2_btn |= MAI2_IO_GAMEBTN_8;
    }

    if (GetAsyncKeyState(mai2_io_cfg.vk_2p_btn[8])) {
        mai2_player2_btn |= MAI2_IO_GAMEBTN_SELECT;
    }

    return S_OK;
}

void mai2_io_get_opbtns(uint8_t *opbtn)
{
    if (opbtn != NULL) {
        *opbtn = mai2_opbtn;
    }
}

void mai2_io_get_gamebtns(uint16_t *player1, uint16_t *player2)
{
    if (player1 != NULL) {
        *player1 = mai2_player1_btn;
    }

    if (player2 != NULL ){
        *player2 = mai2_player2_btn;
    }
}

void mai2_io_led_set_color(uint8_t led, uint8_t r, uint8_t g, uint8_t b) {
    dprintf("LED %d: R: %d, G: %d, B: %d\n", led, r, g, b);
}