#include <windows.h>

#include "board/io4.h"
#include "board/sg-reader.h"
#include "board/vfd.h"

#include "hook/process.h"

#include "hooklib/serial.h"
#include "hooklib/spike.h"

#include "mai2hook/config.h"
#include "mai2hook/io4.h"
#include "mai2hook/mai2-dll.h"
#include "mai2hook/unity.h"
#include "mai2hook/led.h"

#include "platform/platform.h"

#include "util/dprintf.h"

static HMODULE mai2_hook_mod;
static process_entry_t mai2_startup;
static struct mai2_hook_config mai2_hook_cfg;

/* This hook is based on mu3hook, with leaked mai2hook i/o codes. */

static DWORD CALLBACK mai2_pre_startup(void)
{
    HRESULT hr;

    dprintf("--- Begin mai2_pre_startup ---\n");

    /* Load config */

    mai2_hook_config_load(&mai2_hook_cfg, L".\\segatools.ini");

    /* Hook Win32 APIs */

    dvd_hook_init(&mai2_hook_cfg.dvd, mai2_hook_mod);
    gfx_hook_init(&mai2_hook_cfg.gfx, mai2_hook_mod);
    serial_hook_init();

    /* Initialize emulation hooks */

    hr = platform_hook_init(
            &mai2_hook_cfg.platform,
            "SDEZ",
            "ACA1",
            mai2_hook_mod);

    if (FAILED(hr)) {
        goto fail;
    }

    hr = sg_reader_hook_init(&mai2_hook_cfg.aime, 1, mai2_hook_mod);

    if (FAILED(hr)) {
        goto fail;
    }

    hr = vfd_hook_init(2);

    if (FAILED(hr)) {
        goto fail;
    }

    hr = mai2_dll_init(&mai2_hook_cfg.dll, mai2_hook_mod);

    if (FAILED(hr)) {
        goto fail;
    }

    hr = mai2_io4_hook_init(&mai2_hook_cfg.io4);

    if (FAILED(hr)) {
        goto fail;
    }

    hr = mai2_led_hook_init(&mai2_hook_cfg.led, 1, mai2_hook_mod);

    if (FAILED(hr)) {
        goto fail;
    }

    /* Initialize Unity native plugin DLL hooks

       There seems to be an issue with other DLL hooks if `LoadLibraryW` is
       hooked earlier in the `mai2hook` initialization. */

    unity_hook_init();

    /* Initialize debug helpers */

    spike_hook_init(L".\\segatools.ini");

    dprintf("---  End  mai2_pre_startup ---\n");

    /* Jump to EXE start address */

    return mai2_startup();

fail:
    ExitProcess(EXIT_FAILURE);
}

BOOL WINAPI DllMain(HMODULE mod, DWORD cause, void *ctx)
{
    HRESULT hr;

    if (cause != DLL_PROCESS_ATTACH) {
        return TRUE;
    }

    mai2_hook_mod = mod;

    hr = process_hijack_startup(mai2_pre_startup, &mai2_startup);

    if (!SUCCEEDED(hr)) {
        dprintf("Failed to hijack process startup: %x\n", (int) hr);
    }

    return SUCCEEDED(hr);
}
