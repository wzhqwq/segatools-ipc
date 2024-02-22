#include <windows.h>
#include <stdbool.h>

#include "sinmaihook/led-cmd.h"

#include "util/dprintf.h"

static int led_read_package(struct iobuf *buffer, struct led_pkg *package);
static bool led_write_package(struct iobuf *buffer, const struct led_pkg *package);
static int led_handle_req(const struct led_pkg *package, struct led_ack *ack, struct led_board *board);

static int handle_set_timeout(const struct led_set_timeout_cmd_req *req, struct led_board *board);
static int handle_set_gs_single(const struct led_set_single_cmd_req *req, struct led_board *board);
static int handle_set_gs_multi(const struct led_set_multi_cmd_req *req, struct led_board *board);
static int handle_set_gs_multi_fade(const struct led_set_multi_cmd_req *req, struct led_board *board);
static int handle_set_fet(const struct led_set_fet_cmd_req *req, struct led_board *board);
static int handle_dc_update(struct led_board *board);
static int handle_gs_update(struct led_board *board);
static int handle_set_dc(const struct led_set_dc_cmd_req *req, struct led_board *board);
static int handle_set_rom(const struct led_set_rom_cmd_req *req, struct led_board *board);
static int handle_get_rom(const struct led_get_rom_cmd_req *req, struct led_get_rom_cmd_ack *ack, struct led_board *board);
static int handle_set_all(const struct led_set_all_cmd_req *req, struct led_board *board);
static int handle_get_board_info(struct led_get_board_info_cmd_ack *ack, struct led_board *board);
static int handle_get_board_status(struct led_get_board_status_cmd_ack *ack, struct led_board *board);
static int handle_get_firmware_sum(struct led_get_firm_sum_cmd_ack *ack, struct led_board *board);
static int handle_get_protocol_ver(struct led_get_protocol_ver_cmd_ack *ack, struct led_board *board);
static int handle_boot(struct led_board *board);

HRESULT led_handle_write(struct led_board *board)
{
    int ack_status = LED_ACK_OK;

    struct led_pkg package_in, package_out;
    struct led_req *req = (struct led_req *)&package_in.payload;
    struct led_ack *ack = (struct led_ack *)&package_out.payload;

    package_out.hdr.src = LED_NODE_ID;
    package_out.hdr.dst = LED_HOST_ID;

    while (
        board->uart->written.pos &&
        board->uart->readable.pos < board->uart->readable.nbytes - MAX_LED_ENCODED_SIZE)
    {
        uint8_t ack_length = 3;
        ack->report = LED_ACK_RETURN_OK;

        if ((ack_status = led_read_package(&board->uart->written, &package_in)) != LED_ACK_OK)
            goto led_handle_write_end;
        if ((ack_status = led_handle_req(&package_in, ack, board)) != LED_ACK_OK)
            goto led_handle_write_end;

        switch (req->cmd)
        {
        case LED_CMD_GET_ROM:
            ack_length += sizeof(struct led_get_rom_cmd_ack);
        case LED_CMD_GET_BOARD_INFO:
            ack_length += sizeof(struct led_get_board_info_cmd_ack *);
        case LED_CMD_GET_BOARD_STATUS:
            ack_length += sizeof(struct led_get_board_status_cmd_ack *);
        case LED_CMD_GET_FIRMWARE_SUM:
            ack_length += sizeof(struct led_get_firm_sum_cmd_ack *);
        case LED_CMD_GET_PROTOCOL_VER:
            ack_length += sizeof(struct led_get_protocol_ver_cmd_ack *);
        }

    led_handle_write_end:
        if (ack_status == LED_ACK_DROP)
            continue;

        if (ack_status != LED_ACK_OK)
            ack->report = LED_ACK_RETURN_INVALID;
        ack->status = ack_status;
        ack->cmd = req->cmd;

        package_out.hdr.length = ack_length;

        if (!led_write_package(&board->uart->readable, &package_out))
            dprintf("Failed to write package (encoding error), it will be dropped\n");
    }

    return S_OK;
}

static int led_handle_req(const struct led_pkg *package, struct led_ack *ack, struct led_board *board)
{
    if (package->hdr.src != LED_HOST_ID)
    {
        dprintf("Invalid source: %d\n", package->hdr.src);
        return LED_ACK_FRAME_ERROR;
    }
    if (package->hdr.dst != LED_NODE_ID)
    {
        dprintf("Invalid destination: %d\n", package->hdr.dst);
        return LED_ACK_FRAME_ERROR;
    }

    struct led_req *req = (struct led_req *)&package->payload;
    uint8_t body_size = package->hdr.length - offsetof(struct led_req, body);

    switch (req->cmd)
    {
    case LED_CMD_RESET:
        dprintf("Received reset command\n");
        board->uart->readable.pos = 0;
        board->uart->written.pos = 0;
        memset(&board->eep_rom, 0, sizeof(board->eep_rom));
        // TODO: reset other things
        break;
    case LED_CMD_SET_TIMEOUT:
        if (body_size != sizeof(struct led_set_timeout_cmd_req))
            return LED_ACK_FRAME_ERROR;
        dprintf("Received set timeout command\n");
        ack->report = handle_set_timeout((struct led_set_timeout_cmd_req *)req->body, board);
        break;
    case LED_CMD_SET_GS_SINGLE:
        if (body_size != sizeof(struct led_set_single_cmd_req))
            return LED_ACK_FRAME_ERROR;
        dprintf("Received set gs single command\n");
        ack->report = handle_set_gs_single((struct led_set_single_cmd_req *)req->body, board);
        break;
    case LED_CMD_SET_GS_MULTI:
        if (body_size != sizeof(struct led_set_multi_cmd_req))
            return LED_ACK_FRAME_ERROR;
        dprintf("Received set gs multi command\n");
        ack->report = handle_set_gs_multi((struct led_set_multi_cmd_req *)req->body, board);
        break;
    case LED_CMD_SET_GS_MULTI_FADE:
        if (body_size != sizeof(struct led_set_multi_cmd_req))
            return LED_ACK_FRAME_ERROR;
        dprintf("Received set gs multi fade command\n");
        ack->report = handle_set_gs_multi_fade((struct led_set_multi_cmd_req *)req->body, board);
        break;
    case LED_CMD_SET_FET:
        if (body_size != sizeof(struct led_set_fet_cmd_req))
            return LED_ACK_FRAME_ERROR;
        dprintf("Received set fet command\n");
        ack->report = handle_set_fet((struct led_set_fet_cmd_req *)req->body, board);
        break;
    case LED_CMD_DC_UPDATE:
        dprintf("Received dc update command\n");
        ack->report = handle_dc_update(board);
        break;
    case LED_CMD_GS_UPDATE:
        dprintf("Received gs update command\n");
        ack->report = handle_gs_update(board);
        break;
    case LED_CMD_SET_DC:
        if (body_size != sizeof(struct led_set_dc_cmd_req))
            return LED_ACK_FRAME_ERROR;
        dprintf("Received set dc command\n");
        ack->report = handle_set_dc((struct led_set_dc_cmd_req *)req->body, board);
        break;
    case LED_CMD_SET_ROM:
        if (body_size != sizeof(struct led_set_rom_cmd_req))
            return LED_ACK_FRAME_ERROR;
        dprintf("Received set rom command\n");
        ack->report = handle_set_rom((struct led_set_rom_cmd_req *)req->body, board);
        break;
    case LED_CMD_GET_ROM:
        if (body_size != sizeof(struct led_get_rom_cmd_req))
            return LED_ACK_FRAME_ERROR;
        dprintf("Received get rom command\n");
        ack->report = handle_get_rom((struct led_get_rom_cmd_req *)req->body, (struct led_get_rom_cmd_ack *)ack->body, board);
        break;
    case LED_CMD_ENABLE_RESPONSE:
        dprintf("Received enable response command\n");
        board->response_enabled = true;
        break;
    case LED_CMD_DISABLE_RESPONSE:
        dprintf("Received disable response command\n");
        board->response_enabled = false;
        break;
    case LED_CMD_SET_ALL:
        if (body_size != sizeof(struct led_set_all_cmd_req))
            return LED_ACK_FRAME_ERROR;
        dprintf("Received set all command\n");
        ack->report = handle_set_all((struct led_set_all_cmd_req *)req->body, board);
        break;
    case LED_CMD_GET_BOARD_INFO:
        dprintf("Received get board info command\n");
        ack->report = handle_get_board_info((struct led_get_board_info_cmd_ack *)ack->body, board);
        break;
    case LED_CMD_GET_BOARD_STATUS:
        dprintf("Received get board status command\n");
        ack->report = handle_get_board_status((struct led_get_board_status_cmd_ack *)ack->body, board);
        break;
    case LED_CMD_GET_FIRMWARE_SUM:
        dprintf("Received get firmware sum command\n");
        ack->report = handle_get_firmware_sum((struct led_get_firm_sum_cmd_ack *)ack->body, board);
        break;
    case LED_CMD_GET_PROTOCOL_VER:
        dprintf("Received get protocol ver command\n");
        ack->report = handle_get_protocol_ver((struct led_get_protocol_ver_cmd_ack *)ack->body, board);
        break;
    case LED_CMD_BOOT:
        dprintf("Received boot command\n");
        ack->report = handle_boot(board);
        break;
    default:
        dprintf("Unimpl LED command %02x\n", req->cmd);
        ack->report = LED_ACK_RETURN_UNKNOWN_CMD;
    }

    return LED_ACK_OK;
}

static int led_read_package(struct iobuf *buffer, struct led_pkg *package)
{
    uint8_t *buf = buffer->bytes;
    uint8_t *buf_end = buffer->bytes + buffer->pos;
    uint8_t *pkg_data = (uint8_t *)package;

    int result = LED_ACK_OK;

    while (*buf != LED_SYNC)
    {
        buf++;
        if (buf == buf_end)
        {
            buffer->pos = 0;
            return LED_ACK_DROP;
        }
    }

    int read_nbytes = 1;
    pkg_data[0] = LED_SYNC;
    for (uint8_t *p = buf + 1; p < buf_end; p++)
    {
        if (*p == LED_SYNC)
        {
            if (read_nbytes < sizeof(struct led_hdr))
                result = LED_ACK_DROP;
            buf_end = p;
            break;
        }
        if (*p == LED_ESCAPE)
        {
            p++;
            if (p == buf_end)
            {
                result = read_nbytes < sizeof(struct led_hdr) ? LED_ACK_DROP : LED_ACK_FRAME_ERROR;
                break;
            }
            pkg_data[read_nbytes++] = *p + 1;
        }
        else
            pkg_data[read_nbytes++] = *p;
    }

    // flush buffer that already read [buffer->bytes, buf_end)
    size_t flush_nbytes = buf_end - buffer->bytes;
    memmove(buffer->bytes, buf_end, buffer->pos - flush_nbytes);
    buffer->pos -= flush_nbytes;

    if (result != LED_ACK_OK)
        return result;

    // check read length
    if (read_nbytes < sizeof(struct led_hdr))
        return LED_ACK_DROP;
    if (package->hdr.length != read_nbytes - sizeof(struct led_hdr) - 1)
        return LED_ACK_FRAME_ERROR;

    // check sum
    uint8_t sum = 0;
    for (int i = 1; i < read_nbytes - 1; i++)
        sum += pkg_data[i];
    if (sum != pkg_data[read_nbytes - 1])
        return LED_ACK_SUM_ERROR;

    return LED_ACK_OK;
}

static bool led_write_package(struct iobuf *buffer, const struct led_pkg *package)
{
    // This size does noe include the sum byte
    size_t package_size = sizeof(struct led_hdr) + package->hdr.length;

    uint8_t data[MAX_LED_ENCODED_SIZE];
    size_t length = 0;

    data[length++] = LED_SYNC;
    for (size_t i = 1; i < package_size; i++)
    {
        uint8_t byte = ((uint8_t *)package)[i];
        if (byte == LED_SYNC || byte == LED_ESCAPE)
        {
            if (length + 2 > MAX_LED_ENCODED_SIZE)
                return false;
            data[length++] = LED_ESCAPE;
            data[length++] = byte - 1;
        }
        else
        {
            if (length + 1 > MAX_LED_ENCODED_SIZE)
                return false;
            data[length++] = byte;
        }
    }

    if (length + 1 > MAX_LED_ENCODED_SIZE)
        return false;
    uint8_t sum = 0;
    for (size_t i = 1; i < package_size; i++)
        sum += ((uint8_t *)package)[i];
    data[length++] = sum;

    // There must be at least MAX_LED_ENCODED_SIZE bytes available in the buffer
    assert(iobuf_write(buffer, data, length) == S_OK);

    return true;
}

static int handle_set_timeout(const struct led_set_timeout_cmd_req *req, struct led_board *board)
{
    board->timeout = req->timeout;
    return LED_ACK_RETURN_OK;
}
static int handle_set_gs_single(const struct led_set_single_cmd_req *req, struct led_board *board)
{
    return LED_ACK_RETURN_OK;
}
static int handle_set_gs_multi(const struct led_set_multi_cmd_req *req, struct led_board *board)
{
    return LED_ACK_RETURN_OK;
}
static int handle_set_gs_multi_fade(const struct led_set_multi_cmd_req *req, struct led_board *board)
{
    return LED_ACK_RETURN_OK;
}
static int handle_set_fet(const struct led_set_fet_cmd_req *req, struct led_board *board)
{
    return LED_ACK_RETURN_OK;
}
static int handle_dc_update(struct led_board *board)
{
    return LED_ACK_RETURN_OK;
}
static int handle_gs_update(struct led_board *board)
{
    return LED_ACK_RETURN_OK;
}
static int handle_set_dc(const struct led_set_dc_cmd_req *req, struct led_board *board)
{
    return LED_ACK_RETURN_OK;
}
static int handle_set_rom(const struct led_set_rom_cmd_req *req, struct led_board *board)
{
    if (req->addr < 0 || req->addr >= sizeof(board->eep_rom))
        return LED_ACK_RETURN_WRONG_PARAM;
    ((uint8_t *)&board->eep_rom)[req->addr] = req->data;
    return LED_ACK_RETURN_OK;
}
static int handle_get_rom(const struct led_get_rom_cmd_req *req, struct led_get_rom_cmd_ack *ack, struct led_board *board)
{
    if (req->addr < 0 || req->addr >= sizeof(board->eep_rom))
        return LED_ACK_RETURN_WRONG_PARAM;
    ack->data = ((uint8_t *)&board->eep_rom)[req->addr];
    return LED_ACK_RETURN_OK;
}
static int handle_set_all(const struct led_set_all_cmd_req *req, struct led_board *board)
{
    return LED_ACK_RETURN_OK;
}
static int handle_get_board_info(struct led_get_board_info_cmd_ack *ack, struct led_board *board)
{
    strcpy(ack->board_no, LED_BOARD_NO);
    ack->divider_ff = 0xff;
    ack->firm_revision = LED_FIRM_VER;
    return LED_ACK_RETURN_OK;
}
static int handle_get_board_status(struct led_get_board_status_cmd_ack *ack, struct led_board *board)
{
    ack->timeout = board->timeout;
    // TODO: other status
    return LED_ACK_RETURN_OK;
}
static int handle_get_firmware_sum(struct led_get_firm_sum_cmd_ack *ack, struct led_board *board)
{
    ack->firm_sum_lower = LED_FIRM_SUM & 0xff;
    ack->firm_sum_upper = (LED_FIRM_SUM >> 8) & 0xff;
    return LED_ACK_RETURN_OK;
}
static int handle_get_protocol_ver(struct led_get_protocol_ver_cmd_ack *ack, struct led_board *board)
{
    ack->appli_mode = 1;
    // TODO: other info
    return LED_ACK_RETURN_OK;
}
static int handle_boot(struct led_board *board)
{
    return LED_ACK_RETURN_OK;
}
