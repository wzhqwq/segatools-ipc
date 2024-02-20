#include <stdbool.h>
#include <string.h>

#include "sinmaihook/dynamic-inject.h"

#include "util/dprintf.h"

static FARPROC my_GetProcAddress(HMODULE module, const char *name);
static FARPROC(WINAPI *next_GetProcAddress)(HMODULE module, const char *name);

static const struct hook_symbol dyn_inject_kernel32_syms[] = {
    {
        .name = "GetProcAddress",
        .patch = my_GetProcAddress,
        .link = (void **)&next_GetProcAddress,
    },
};

static struct hook_symbol dynamic_injected_syms[50];
static size_t dynamic_injected_count = 0;

void dyn_inject_hook_init(void)
{
    dyn_inject_insert_hooks(NULL);
}

void dyn_inject_insert_hooks(HMODULE target)
{
    hook_table_apply(
        target,
        "kernel32.dll",
        dyn_inject_kernel32_syms,
        _countof(dyn_inject_kernel32_syms));
}

void dyn_inject_push_syms(const struct hook_symbol *syms, size_t count)
{
    memcpy(
        &dynamic_injected_syms[dynamic_injected_count],
        syms,
        count * sizeof(struct hook_symbol));
    dynamic_injected_count += count;
}

static FARPROC my_GetProcAddress(HMODULE module, const char *name)
{
    FARPROC result;

    result = next_GetProcAddress(module, name);

    if (result != NULL)
    {
        // dprintf("GetProcAddress: %s\n", name);
        for (size_t i = 0; i < dynamic_injected_count; i++)
        {
            if (strcmp(dynamic_injected_syms[i].name, name) == 0)
            {
                if (dynamic_injected_syms[i].link != NULL && *dynamic_injected_syms[i].link == NULL) {
                    *dynamic_injected_syms[i].link = result;
                }
                // dprintf("Patched: %s\n", name);
                return dynamic_injected_syms[i].patch;
            }
        }
    }

    return result;
}
