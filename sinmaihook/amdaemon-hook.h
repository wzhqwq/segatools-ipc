#include <windows.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define QUICK_HOOK_DEF(NAME, RET, ...) \
static RET (*real_##NAME) (__VA_ARGS__); \
static RET my_##NAME (__VA_ARGS__)

struct hook_symbol_ex {
    const char *name;
    void *patch;
    void **link;
    // must be at least 10, best at least 14
    int overwrite_size;
    uint8_t trampoline[40];
    int function_size;
    // if only_call < overwrite_size, must move forward the only_call and rest code
    int only_call;
};

#define QUICK_HOOK_SYM(NAME, SIZE) \
    {\
        .name = #NAME, \
        .patch = my_##NAME, \
        .link = (void *) &real_##NAME, \
        .overwrite_size = SIZE, \
        .only_call = -1, \
    }
#define QUICK_HOOK_SYM_WITH_CALL(NAME, SIZE, CALL, FN_SIZE) \
    {\
        .name = #NAME, \
        .patch = my_##NAME, \
        .link = (void *) &real_##NAME, \
        .overwrite_size = SIZE, \
        .only_call = CALL, \
        .function_size = FN_SIZE, \
    }

#define QUICK_HOOK_IMPL_NORETURN(NAME, PARAM_PATTERN, ...) \
{\
    real_##NAME (__VA_ARGS__); \
    dprintf(#NAME "(" PARAM_PATTERN ")", __VA_ARGS__); \
    dprintf(" = void\n"); \
}

#define QUICK_HOOK_IMPL_NOARGS(NAME, RET, RET_CODE) \
{\
    RET ret = real_##NAME (); \
    dprintf(#NAME "()"); \
    RET_CODE \
    return ret; \
}

#define QUICK_HOOK_IMPL(NAME, RET, RET_CODE, PARAM_PATTERN, ...) \
{\
    RET ret = real_##NAME (__VA_ARGS__); \
    dprintf(#NAME "(" PARAM_PATTERN ")", __VA_ARGS__); \
    RET_CODE \
    return ret; \
}

#define RETURN_NORMAL(RET_PATTERN) dprintf(" = " RET_PATTERN "\n", ret);
#define RETURN_STRING() dprintf(" = %s\n", ret);
#define RETURN_POINTER(RET_PATTERN) dprintf(" = %p (" RET_PATTERN ")\n", ret, *ret);

void amdaemon_hook_patch(HMODULE target);

FARPROC amdeamon_hook_patch_one(FARPROC src, const char *name);
