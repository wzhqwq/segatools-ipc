#include <windows.h>

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include "mai2hook/mai2-dll.h"
#include "mai2hook/led.h"
#include "board/sg-led.h"
#include "board/sg-reader.h"

#include "hook/iohook.h"

#include "hooklib/uart.h"

#include "util/dprintf.h"
#include "util/dump.h"

// static HRESULT mai2_led_handle_irp(struct irp *irp);
// static HRESULT mai2_led_handle_irp_locked(struct irp *irp);
static void mai2_led_set_color(void *ctx, uint8_t r, uint8_t g, uint8_t b);

static const struct sg_led_ops mai2_led_ops = {
    .set_color          = mai2_led_set_color,
};

// static CRITICAL_SECTION mai2_led_lock;
// static struct uart mai2_led_uart;
// static uint8_t mai2_led_written_bytes[520];
// static uint8_t mai2_led_readable_bytes[520];
static struct sg_led reader_led;

HRESULT mai2_led_hook_init(
        const struct led_config *cfg,
        unsigned int port_no,
        HINSTANCE self)
{
    assert(cfg != NULL);
    assert(self != NULL);

    if (!cfg->enable) {
        return S_FALSE;
    }

    sg_led_init(&reader_led, 0x08, &mai2_led_ops, (void *) 1);
    sg_reader_custom_led = &reader_led;

    // InitializeCriticalSection(&mai2_led_lock);

    // uart_init(&mai2_led_uart, port_no);
    // mai2_led_uart.written.bytes = mai2_led_written_bytes;
    // mai2_led_uart.written.nbytes = sizeof(mai2_led_written_bytes);
    // mai2_led_uart.readable.bytes = mai2_led_readable_bytes;
    // mai2_led_uart.readable.nbytes = sizeof(mai2_led_readable_bytes);

    // return iohook_push_handler(mai2_led_handle_irp);
    return S_OK;
}

static void mai2_led_set_color(void *ctx, uint8_t r, uint8_t g, uint8_t b)
{
    mai2_dll.led_set_color((uint8_t) ctx, r, g, b);
}
