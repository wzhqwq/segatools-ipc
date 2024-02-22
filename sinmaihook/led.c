#include <windows.h>
#include <ntstatus.h>

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#include "sinmaihook/iohook-dynamic.h"
#include "sinmaihook/led.h"
#include "sinmaihook/led-cmd.h"

#include "hooklib/uart.h"

#include "util/dprintf.h"
#include "util/dump.h"

static HRESULT led_handle_irp(struct irp *irp);
static HRESULT led_handle_irp_locked(struct irp *irp, struct led_board *board);

static CRITICAL_SECTION led_lock_1p, led_lock_2p;
static struct uart led_uart_1p, led_uart_2p;
static struct led_board led_board_1p = {
    .player = 1,
    .uart = &led_uart_1p,
};
static struct led_board led_board_2p = {
    .player = 2,
    .uart = &led_uart_2p,
};

static uint8_t led_written_bytes_1p[20];
static uint8_t led_written_bytes_2p[20];
static uint8_t led_readable_bytes_1p[100];
static uint8_t led_readable_bytes_2p[100];

HRESULT led_hook_init()
{
    InitializeCriticalSection(&led_lock_1p);
    InitializeCriticalSection(&led_lock_2p);

    uart_init(&led_uart_1p, 21);
    led_uart_1p.written.bytes = led_written_bytes_1p;
    led_uart_1p.written.nbytes = sizeof(led_written_bytes_1p);
    led_uart_1p.readable.bytes = led_readable_bytes_1p;
    led_uart_1p.readable.nbytes = sizeof(led_readable_bytes_1p);
    uart_init(&led_uart_2p, 23);
    led_uart_2p.written.bytes = led_written_bytes_2p;
    led_uart_2p.written.nbytes = sizeof(led_written_bytes_2p);
    led_uart_2p.readable.bytes = led_readable_bytes_2p;
    led_uart_2p.readable.nbytes = sizeof(led_readable_bytes_2p);

    return iohook_push_handler(led_handle_irp);
}

static HRESULT led_handle_irp(struct irp *irp)
{
    HRESULT hr;
    CRITICAL_SECTION *led_lock;
    struct led_board *board;

    assert(irp != NULL);

    bool is_1p = uart_match_irp(&led_uart_1p, irp);
    bool is_2p = uart_match_irp(&led_uart_2p, irp);

    if (!is_1p && !is_2p)
        return iohook_invoke_next(irp);

    led_lock = is_1p ? &led_lock_1p : &led_lock_2p;
    board = is_1p ? &led_board_1p : &led_board_2p;

    // The .NET always opens serial port with FILE_FLAG_OVERLAPPED,
    // but calls ReadFile with lpNumberOfBytesRead provided (WriteFile too),
    // which disturbs iohook_overlapped_result().
    // Set the flag to remind the handler to remove lpNumberOfBytesRead.
    irp->open_flags |= FILE_FLAG_OVERLAPPED;

    EnterCriticalSection(led_lock);
    hr = led_handle_irp_locked(irp, board);
    LeaveCriticalSection(led_lock);

    return hr;
}

static HRESULT led_handle_irp_locked(struct irp *irp, struct led_board *board)
{
    HRESULT hr;

#if 0
    if (irp->op == IRP_OP_WRITE)
    {
        dprintf("Led board %dp write:\n", board->player);
        dump_const_iobuf(&irp->write);
    }
#endif

#if 0
    if (irp->op == IRP_OP_READ && player_uart->readable.pos > 0)
    {
        dprintf("Led board %dp will read:\n", board->player);
        dump_iobuf(&player_uart->readable);
    }
#endif

    if (irp->op == IRP_OP_OPEN)
    {
        dprintf("Led board %dP opened\n", board->player);
    }
    if (irp->op == IRP_OP_CLOSE)
    {
        dprintf("Led board %dP closed\n", board->player);
    }

    hr = uart_handle_irp(board->uart, irp);

    if (FAILED(hr) || irp->op != IRP_OP_WRITE)
    {
        return hr;
    }

    hr = led_handle_write(board);

    return hr;
}

