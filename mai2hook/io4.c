#include <windows.h>

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "mai2hook/io4.h"
#include "mai2hook/mai2-dll.h"

#include "util/dprintf.h"

static HRESULT mai2_io4_poll(void *ctx, struct io4_state *state);
static HRESULT mai2_io4_handle_gpio(const uint8_t *payload);
static HRESULT mai2_io4_handle_unique_io(const uint8_t *payload);

static const struct io4_ops mai2_io4_ops = {
    .poll = mai2_io4_poll,
    .handle_usb_gpio = mai2_io4_handle_gpio,
    .handle_usb_pwm = NULL,
    .handle_usb_unique_io = mai2_io4_handle_unique_io,
};

struct io4_state rolling_state;

HRESULT mai2_io4_hook_init(const struct io4_config *cfg)
{
    HRESULT hr;

    assert(mai2_dll.init != NULL);

    hr = io4_hook_init(cfg, &mai2_io4_ops, NULL);

    if (FAILED(hr)) {
        return hr;
    }

    return mai2_dll.init();
}

static HRESULT mai2_io4_poll(void *ctx, struct io4_state *state)
{
    uint8_t opbtn;
    uint16_t player1;
    uint16_t player2;
    HRESULT hr;

    assert(mai2_dll.poll != NULL);
    assert(mai2_dll.get_opbtns != NULL);
    assert(mai2_dll.get_gamebtns != NULL);

    memset(state, 0, sizeof(*state));

    hr = mai2_dll.poll();

    if (FAILED(hr)) {
        return hr;
    }

    opbtn = 0;
    player1 = 0;
    player2 = 0;

    mai2_dll.get_opbtns(&opbtn);
    mai2_dll.get_gamebtns(&player1, &player2);

    if (opbtn & MAI2_IO_OPBTN_TEST) {
        state->buttons[0] |= IO4_BUTTON_TEST;
    }

    if (opbtn & MAI2_IO_OPBTN_SERVICE) {
        state->buttons[0] |= IO4_BUTTON_SERVICE;
    }

    // Buttons around screen are active-low, select button is active-high

    // Player 1

    if (!(player1 & MAI2_IO_GAMEBTN_1)) {
        state->buttons[0] |= 1 << 2;
    }

    if (!(player1 & MAI2_IO_GAMEBTN_2)) {
        state->buttons[0] |= 1 << 3;
    }

    if (!(player1 & MAI2_IO_GAMEBTN_3)) {
        state->buttons[0] |= 1 << 0;
    }

    if (!(player1 & MAI2_IO_GAMEBTN_4)) {
        state->buttons[0] |= 1 << 15;
    }

    if (!(player1 & MAI2_IO_GAMEBTN_5)) {
        state->buttons[0] |= 1 << 14;
    }

    if (!(player1 & MAI2_IO_GAMEBTN_6)) {
        state->buttons[0] |= 1 << 13;
    }

    if (!(player1 & MAI2_IO_GAMEBTN_7)) {
        state->buttons[0] |= 1 << 12;
    }

    if (!(player1 & MAI2_IO_GAMEBTN_8)) {
        state->buttons[0] |= 1 << 11;
    }

    if (player1 & MAI2_IO_GAMEBTN_SELECT) {
        state->buttons[0] |= 1 << 1;
    }

    // Player 2

    if (!(player2 & MAI2_IO_GAMEBTN_1)) {
        state->buttons[1] |= 1 << 2;
    }

    if (!(player2 & MAI2_IO_GAMEBTN_2)) {
        state->buttons[1] |= 1 << 3;
    }

    if (!(player2 & MAI2_IO_GAMEBTN_3)) {
        state->buttons[1] |= 1 << 0;
    }

    if (!(player2 & MAI2_IO_GAMEBTN_4)) {
        state->buttons[1] |= 1 << 15;
    }

    if (!(player2 & MAI2_IO_GAMEBTN_5)) {
        state->buttons[1] |= 1 << 14;
    }

    if (!(player2 & MAI2_IO_GAMEBTN_6)) {
        state->buttons[1] |= 1 << 13;
    }

    if (!(player2 & MAI2_IO_GAMEBTN_7)) {
        state->buttons[1] |= 1 << 12;
    }

    if (!(player2 & MAI2_IO_GAMEBTN_8)) {
        state->buttons[1] |= 1 << 11;
    }

    if (player2 & MAI2_IO_GAMEBTN_SELECT) {
        state->buttons[1] |= 1 << 4;
    }

    return S_OK;
}

static HRESULT mai2_io4_handle_gpio(const uint8_t *payload)
{
    int state = payload[0] << 8 | payload[1];

    return S_OK;
}

static HRESULT mai2_io4_handle_unique_io(const uint8_t *payload)
{
    return S_OK;
}