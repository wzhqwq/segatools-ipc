#include <windows.h>

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include "sinmaihook/iohook-dynamic.h"

#include "hooklib/uart.h"

#include "util/dprintf.h"
#include "util/dump.h"

static HRESULT touch_panel_handle_irp(struct irp *irp);
static HRESULT touch_panel_handle_irp_locked(struct irp *irp, int player, struct uart *player_uart);

static CRITICAL_SECTION touch_panel_lock;
static struct uart touch_panel_uart_1p, touch_panel_uart_2p;
static uint8_t touch_panel_written_bytes[520];
static uint8_t touch_panel_readable_bytes[520];

HRESULT touch_panel_hook_init()
{
    InitializeCriticalSection(&touch_panel_lock);

    uart_init(&touch_panel_uart_1p, 3);
    touch_panel_uart_1p.written.bytes = touch_panel_written_bytes;
    touch_panel_uart_1p.written.nbytes = sizeof(touch_panel_written_bytes);
    touch_panel_uart_1p.readable.bytes = touch_panel_readable_bytes;
    touch_panel_uart_1p.readable.nbytes = sizeof(touch_panel_readable_bytes);
    uart_init(&touch_panel_uart_2p, 4);
    touch_panel_uart_2p.written.bytes = touch_panel_written_bytes;
    touch_panel_uart_2p.written.nbytes = sizeof(touch_panel_written_bytes);
    touch_panel_uart_2p.readable.bytes = touch_panel_readable_bytes;
    touch_panel_uart_2p.readable.nbytes = sizeof(touch_panel_readable_bytes);

    return iohook_push_handler(touch_panel_handle_irp);
}

static HRESULT touch_panel_handle_irp(struct irp *irp)
{
    HRESULT hr;

    assert(irp != NULL);

    if (irp->op == IRP_OP_OPEN && wcsstr(irp->open_filename, L"COM"))
    {
        dprintf("Open: %S\n", irp->open_filename);
    }

    bool is_1p = uart_match_irp(&touch_panel_uart_1p, irp);
    bool is_2p = uart_match_irp(&touch_panel_uart_2p, irp);

    if (!is_1p && !is_2p)
    {
        return iohook_invoke_next(irp);
    }

    EnterCriticalSection(&touch_panel_lock);
    hr = touch_panel_handle_irp_locked(irp, is_1p ? 1 : 2, is_1p ? &touch_panel_uart_1p : &touch_panel_uart_2p);
    LeaveCriticalSection(&touch_panel_lock);

    return hr;
}

static HRESULT touch_panel_handle_irp_locked(struct irp *irp, int player, struct uart *player_uart)
{
    HRESULT hr;

#if 1
    if (irp->op == IRP_OP_WRITE)
    {
        dprintf("Touch screen %dp write:\n", player);
        dump_const_iobuf(&irp->write);
    }
#endif

#if 1
    if (irp->op == IRP_OP_READ)
    {
        dprintf("Touch screen %dp will read:\n", player);
        dump_iobuf(&player_uart->readable);
    }
#endif

    if (irp->op == IRP_OP_OPEN)
    {
        dprintf("Touch screen %dp opened\n", player);
    }

    hr = uart_handle_irp(player_uart, irp);

    if (FAILED(hr) || irp->op != IRP_OP_WRITE)
    {
        return hr;
    }

    player_uart->written.pos = 0;

    return hr;
}
