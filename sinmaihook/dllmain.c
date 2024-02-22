#include <windows.h>

#include "hook/process.h"

#include "util/dprintf.h"

#include "sinmaihook/iohook-dynamic.h"
#include "sinmaihook/serial-dynamic.h"

#include "sinmaihook/unity.h"
#include "sinmaihook/dynamic-inject.h"
#include "sinmaihook/touch-panel.h"
#include "sinmaihook/reg.h"
#include "sinmaihook/led.h"

static process_entry_t app_startup;

static DWORD CALLBACK sinmai_pre_startup(void)
{
    dprintf("--- Begin  %s ---\n", __func__);

    dyn_iohook_init();
    dyn_serial_hook_init();

    unity_hook_init();

    dyn_inject_hook_init();

    touch_panel_hook_init();

    reg_hook_init();

    led_hook_init();

    dprintf("---  End  %s ---\n", __func__);

    return app_startup();
}

BOOL WINAPI DllMain(HMODULE mod, DWORD cause, void *ctx)
{
    HRESULT hr;

    if (cause != DLL_PROCESS_ATTACH) {
        return TRUE;
    }

    hr = process_hijack_startup(sinmai_pre_startup, &app_startup);

    if (!SUCCEEDED(hr)) {
        dprintf("Failed to hijack process startup: %x\n", (int) hr);
    }

    return SUCCEEDED(hr);
}
