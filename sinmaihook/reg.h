#include <stddef.h>

struct reg_string_val {
    const wchar_t *name;
    const wchar_t *value;
};

void reg_hook_init(void);
