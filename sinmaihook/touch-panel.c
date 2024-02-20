#include <windows.h>
#include <ntstatus.h>

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#include "sinmaihook/iohook-dynamic.h"
#include "sinmaihook/touch-panel.h"

#include "hooklib/uart.h"

#include "util/dprintf.h"
#include "util/dump.h"

static HRESULT touch_panel_handle_irp(struct irp *irp);
static HRESULT touch_panel_handle_irp_locked(struct irp *irp, struct uart *player_uart, struct touch_panel *panel);
static HRESULT handle_receive(struct uart *player_uart, struct touch_panel *panel);
static HRESULT touch_panel_handle_req(struct uart *player_uart, const struct touch_panel_req *req, struct touch_panel *panel);
static HRESULT touch_panel_send(struct uart *player_uart, struct touch_panel *panel, const void *data);

static CRITICAL_SECTION touch_panel_lock_1p, touch_panel_lock_2p;
static struct uart touch_panel_uart_1p, touch_panel_uart_2p;
static struct touch_panel panel_1p = {
    .player = 1,
    .side = 'L',
};
static struct touch_panel panel_2p = {
    .player = 2,
    .side = 'R',
};

static uint8_t touch_panel_written_bytes_1p[20];
static uint8_t touch_panel_written_bytes_2p[20];
static uint8_t touch_panel_readable_bytes_1p[100];
static uint8_t touch_panel_readable_bytes_2p[100];

HRESULT touch_panel_hook_init()
{
    InitializeCriticalSection(&touch_panel_lock_1p);
    InitializeCriticalSection(&touch_panel_lock_2p);

    uart_init(&touch_panel_uart_1p, 3);
    touch_panel_uart_1p.written.bytes = touch_panel_written_bytes_1p;
    touch_panel_uart_1p.written.nbytes = sizeof(touch_panel_written_bytes_1p);
    touch_panel_uart_1p.readable.bytes = touch_panel_readable_bytes_1p;
    touch_panel_uart_1p.readable.nbytes = sizeof(touch_panel_readable_bytes_1p);
    touch_panel_uart_1p.baud.BaudRate = 9600;
    uart_init(&touch_panel_uart_2p, 4);
    touch_panel_uart_2p.written.bytes = touch_panel_written_bytes_2p;
    touch_panel_uart_2p.written.nbytes = sizeof(touch_panel_written_bytes_2p);
    touch_panel_uart_2p.readable.bytes = touch_panel_readable_bytes_2p;
    touch_panel_uart_2p.readable.nbytes = sizeof(touch_panel_readable_bytes_2p);
    touch_panel_uart_2p.baud.BaudRate = 9600;

    return iohook_push_handler(touch_panel_handle_irp);
}

static HRESULT touch_panel_handle_irp(struct irp *irp)
{
    HRESULT hr;
    CRITICAL_SECTION *touch_panel_lock;
    struct uart *player_uart;
    struct touch_panel *panel;

    assert(irp != NULL);

    bool is_1p = uart_match_irp(&touch_panel_uart_1p, irp);
    bool is_2p = uart_match_irp(&touch_panel_uart_2p, irp);

    if (!is_1p && !is_2p)
        return iohook_invoke_next(irp);

    touch_panel_lock = is_1p ? &touch_panel_lock_1p : &touch_panel_lock_2p;
    player_uart = is_1p ? &touch_panel_uart_1p : &touch_panel_uart_2p;
    panel = is_1p ? &panel_1p : &panel_2p;

    // The .NET always opens serial port with FILE_FLAG_OVERLAPPED,
    // but calls ReadFile with lpNumberOfBytesRead provided (WriteFile too),
    // which disturbs iohook_overlapped_result().
    // Set the flag to remind the handler to remove lpNumberOfBytesRead.
    irp->open_flags |= FILE_FLAG_OVERLAPPED;

    EnterCriticalSection(touch_panel_lock);
    hr = touch_panel_handle_irp_locked(irp, player_uart, panel);
    LeaveCriticalSection(touch_panel_lock);

    return hr;
}

static HRESULT touch_panel_handle_irp_locked(struct irp *irp, struct uart *player_uart, struct touch_panel *panel)
{
    HRESULT hr;

#if 0
    if (irp->op == IRP_OP_WRITE)
    {
        dprintf("Touch screen %dp write:\n", panel->player);
        dump_const_iobuf(&irp->write);
    }
#endif

#if 0
    if (irp->op == IRP_OP_READ && player_uart->readable.pos > 0)
    {
        dprintf("Touch screen %dp will read:\n", panel->player);
        dump_iobuf(&player_uart->readable);
    }
#endif

    if (irp->op == IRP_OP_OPEN)
    {
        dprintf("Touch screen %dP opened\n", panel->player);
    }
    if (irp->op == IRP_OP_CLOSE)
    {
        dprintf("Touch screen %dP closed\n", panel->player);
    }

    hr = uart_handle_irp(player_uart, irp);

    if (FAILED(hr) || irp->op != IRP_OP_WRITE)
    {
        return hr;
    }

    hr = handle_receive(player_uart, panel);
    player_uart->written.pos = 0;

    return hr;
}

static HRESULT handle_receive(struct uart *player_uart, struct touch_panel *panel)
{
    uint8_t *bytes = player_uart->written.bytes;
    size_t nbytes = player_uart->written.pos;
    // dprintf("Touch panel %dP received %d bytes: %s\n", panel->player, nbytes, bytes);

    for (size_t i = 0; i < nbytes; i += sizeof(struct touch_panel_req))
    {
        struct touch_panel_req *req = (struct touch_panel_req *)&bytes[i];
        HRESULT hr = touch_panel_handle_req(player_uart, req, panel);
        if (FAILED(hr))
            return hr;
    }
    return S_OK;
}

static HRESULT touch_panel_handle_req(struct uart *player_uart, const struct touch_panel_req *req, struct touch_panel *panel)
{
    if (req->start_byte != '{' || req->end_byte != '}')
        return E_FAIL;
    switch (req->cmd)
    {
    case TP_CMD_RSET:
        dprintf("Touch panel %dP reset\n", panel->player);
        panel->is_conditioning = false;
        memset(panel->ratio, 50, sizeof(panel->ratio));
        memset(panel->sensitivity, 50, sizeof(panel->sensitivity));
        memset(panel->touch_report, 0, sizeof(panel->touch_report));
        break;
    case TP_CMD_HALT:
        dprintf("Touch panel %dP stop touch transmission\n", panel->player);
        panel->is_conditioning = true;
        break;
    case TP_CMD_STAT:
        dprintf("Touch panel %dP start touch transmission\n", panel->player);
        panel->is_conditioning = false;
        break;
    case TP_CMD_Ratio:
        panel->ratio[req->area - 65] = req->value;
        // dprintf("Touch panel %dP set area %c ratio to %d\n", panel->player, req->area, req->value);
        touch_panel_send(player_uart, panel, (void *)req + 1);
        break;
    case TP_CMD_Sensitivity:
        panel->sensitivity[req->area - 65] = req->value;
        // dprintf("Touch panel %dP set area %c sensitivity to %d\n", panel->player, req->area, req->value);
        touch_panel_send(player_uart, panel, (void *)req + 1);
        break;
    default:
        dprintf("Unimpl touch panel command %02x\n", req->cmd);
        return E_NOTIMPL;
    }

    return S_OK;
}

static HRESULT touch_panel_send(struct uart *player_uart, struct touch_panel *panel, const void *data)
{
    size_t size = panel->is_conditioning ? TP_PONG_SIZE : TP_REPORT_SIZE;
    uint8_t *resp_buf = malloc(size);
    if (!resp_buf)
        return E_OUTOFMEMORY;

    resp_buf[0] = '(';
    resp_buf[size - 1] = ')';
    memcpy(resp_buf + 1, data, size - 2);

    HRESULT result = iobuf_write(&player_uart->readable, resp_buf, size);

    free(resp_buf);

    return result;
}

void touch_panel_update_touch(int player, uint8_t *touch_report)
{
    struct touch_panel *panel = player == 1 ? &panel_1p : &panel_2p;
    CRITICAL_SECTION *touch_panel_lock = player == 1 ? &touch_panel_lock_1p : &touch_panel_lock_2p;
    struct uart *player_uart = player == 1 ? &touch_panel_uart_1p : &touch_panel_uart_2p;

    EnterCriticalSection(touch_panel_lock);
    memcpy(panel->touch_report, touch_report, sizeof(panel->touch_report));
    if (!panel->is_conditioning)
        touch_panel_send(player_uart, panel, panel->touch_report);
    LeaveCriticalSection(touch_panel_lock);
}