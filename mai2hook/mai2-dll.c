#include <windows.h>

#include <assert.h>
#include <stdlib.h>

#include "mai2hook/mai2-dll.h"

#include "util/dll-bind.h"
#include "util/dprintf.h"

const struct dll_bind_sym mai2_dll_syms[] = {
    {
        .sym = "mai2_io_init",
        .off = offsetof(struct mai2_dll, init),
    }, {
        .sym = "mai2_io_poll",
        .off = offsetof(struct mai2_dll, poll),
    }, {
        .sym = "mai2_io_get_opbtns",
        .off = offsetof(struct mai2_dll, get_opbtns),
    }, {
        .sym = "mai2_io_get_gamebtns",
        .off = offsetof(struct mai2_dll, get_gamebtns),
    }, {
        .sym = "mai2_io_led_set_color",
        .off = offsetof(struct mai2_dll, led_set_color),
    }
};

struct mai2_dll mai2_dll;

// Copypasta DLL binding and diagnostic message boilerplate.
// Not much of this lends itself to being easily factored out. Also there
// will be a lot of API-specific branching code here eventually as new API
// versions get defined, so even though these functions all look the same
// now this won't remain the case forever.

HRESULT mai2_dll_init(const struct mai2_dll_config *cfg, HINSTANCE self)
{
    uint16_t (*get_api_version)(void);
    const struct dll_bind_sym *sym;
    HINSTANCE owned;
    HINSTANCE src;
    HRESULT hr;

    assert(cfg != NULL);
    assert(self != NULL);

    if (cfg->path[0] != L'\0') {
        owned = LoadLibraryW(cfg->path);

        if (owned == NULL) {
            hr = HRESULT_FROM_WIN32(GetLastError());
            dprintf("Maimai DX IO: Failed to load IO DLL: %lx: %S\n",
                    hr,
                    cfg->path);

            goto end;
        }

        dprintf("Maimai DX IO: Using custom IO DLL: %S\n", cfg->path);
        src = owned;
    } else {
        owned = NULL;
        src = self;
    }

    get_api_version = (void *) GetProcAddress(src, "mai2_io_get_api_version");

    if (get_api_version != NULL) {
        mai2_dll.api_version = get_api_version();
    } else {
        mai2_dll.api_version = 0x0100;
        dprintf("Custom IO DLL does not expose mai2_io_get_api_version, "
                "assuming API version 1.0.\n"
                "Please ask the developer to update their DLL.\n");
    }

    if (mai2_dll.api_version >= 0x0200) {
        hr = E_NOTIMPL;
        dprintf("Maimai DX IO: Custom IO DLL implements an unsupported "
                "API version (%#04x). Please update Segatools.\n",
                mai2_dll.api_version);

        goto end;
    }

    sym = mai2_dll_syms;
    hr = dll_bind(&mai2_dll, src, &sym, _countof(mai2_dll_syms));

    if (FAILED(hr)) {
        if (src != self) {
            dprintf("Maimai DX IO: Custom IO DLL does not provide function "
                    "\"%s\". Please contact your IO DLL's developer for "
                    "further assistance.\n",
                    sym->sym);

            goto end;
        } else {
            dprintf("Internal error: could not reflect \"%s\"\n", sym->sym);
        }
    }

    owned = NULL;

end:
    if (owned != NULL) {
        FreeLibrary(owned);
    }

    return hr;
}
