#include <windows.h>
#include <stdint.h>

#include "hook/table.h"

void dyn_inject_hook_init(void);
void dyn_inject_insert_hooks(HMODULE target);
void dyn_inject_push_syms(const struct hook_symbol *syms, size_t count);