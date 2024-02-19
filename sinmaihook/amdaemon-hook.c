#include "sinmaihook/amdaemon-hook.h"

#include "util/dprintf.h"

// QUICK_HOOK_DEF(UsbIOGeneralOutput_resetBits, void *, void * a, int8_t b, void * c, void * d)
// QUICK_HOOK_IMPL(UsbIOGeneralOutput_resetBits, void *, RETURN_NORMAL("%p"), "%p, %d, %p, %p", a, b, c, d)
// QUICK_HOOK_DEF(UsbIOGeneralOutput_setBit, void *, void * a, int32_t b, int8_t c, int8_t d)
// QUICK_HOOK_IMPL(UsbIOGeneralOutput_setBit, void *, RETURN_NORMAL("%p"), "%p, %d, %d, %d", a, b, c, d)
// QUICK_HOOK_DEF(UsbIOGeneralOutput_setBits, void *, void * a, void * b, int8_t c, void * d)
// QUICK_HOOK_IMPL(UsbIOGeneralOutput_setBits, void *, RETURN_NORMAL("%p"), "%p, %p, %d, %p", a, b, c, d)

// QUICK_HOOK_DEF(UsbIOAnalogInput_getChannelCount, int32_t, void * a)
// QUICK_HOOK_IMPL(UsbIOAnalogInput_getChannelCount, int32_t, RETURN_NORMAL("%d"), "%p", a)
// QUICK_HOOK_DEF(UsbIOAnalogInput_getValue, int16_t, void * a, int32_t b)
// QUICK_HOOK_IMPL(UsbIOAnalogInput_getValue, int16_t, RETURN_NORMAL("%d"), "%p, %d", a, b)

QUICK_HOOK_DEF(UsbDevice_startReconnect, void *, int16_t a, int16_t b)
QUICK_HOOK_IMPL(UsbDevice_startReconnect, void *, RETURN_NORMAL("%p"), "%d, %d", a, b)
// QUICK_HOOK_DEF(System_getBoardId, void *, void)
// QUICK_HOOK_IMPL_NOARGS(System_getBoardId, void *, RETURN_STRING())

// QUICK_HOOK_DEF(UsbIONode_getGeneralOutput, void*, void* a)
// QUICK_HOOK_IMPL(UsbIONode_getGeneralOutput, void*, RETURN_NORMAL("%p"), "%p", a)

// QUICK_HOOK_DEF(BoardIO_setLedState, void, int32_t a, int8_t b, int8_t c)
// QUICK_HOOK_IMPL_NORETURN(BoardIO_setLedState, "%d, %d, %d", a, b, c)

// QUICK_HOOK_DEF(AnalogInput_getDelta, void, void* a, void* b, int32_t c, void* d)
// QUICK_HOOK_IMPL_NORETURN(AnalogInput_getDelta, "%p, %p, %d, %p", a, b, c, d)
// QUICK_HOOK_DEF(AnalogInput_getValue, void, void* a, void* b, int32_t c, void* d)
// QUICK_HOOK_IMPL_NORETURN(AnalogInput_getValue, "%p, %p, %d, %p", a, b, c, d)
// QUICK_HOOK_DEF(AnalogOutput_getCurrentValue, void, void* a, void* b, int32_t c, void* d)
// QUICK_HOOK_IMPL_NORETURN(AnalogOutput_getCurrentValue, "%p, %p, %d, %p", a, b, c, d)
// QUICK_HOOK_DEF(AnalogOutput_setValue, void, void* a, void* b, int32_t c, void* d, void* e, int8_t f)
// QUICK_HOOK_IMPL_NORETURN(AnalogOutput_setValue, "%p, %p, %d, %p, %p, %d", a, b, c, d, e, f)

QUICK_HOOK_DEF(UsbIO_getNode, void *, int32_t a)
QUICK_HOOK_IMPL(UsbIO_getNode, int *, RETURN_POINTER("%d"), "%d", a)
QUICK_HOOK_DEF(UsbIO_getNodeCount, int32_t, void)
QUICK_HOOK_IMPL_NOARGS(UsbIO_getNodeCount, int32_t, RETURN_NORMAL("%x"))

static void (*real_SwitchOutput_set)(void *sw, wchar_t *name, int32_t bit_index, int8_t value, int8_t e);
/**
 * @param sw            switch address
 * @param name          switch name
 * @param bit_index     bit index to change
 * @param value         new value (0 or 1)
 */
static void my_SwitchOutput_set(void *sw, wchar_t *name, int32_t bit_index, int8_t value, int8_t e)
{
    real_SwitchOutput_set(sw, name, bit_index, value, e);
    dprintf("Switch %p::%S[%d] = %d, %d\n", sw, name, bit_index, value, e);
}
static int64_t (*real_SwitchInput_isOn)(void *sw, wchar_t *name);
static int64_t my_SwitchInput_isOn(void *sw, wchar_t *name)
{
    int64_t ret = real_SwitchInput_isOn(sw, name);
    dprintf("Switch %p::%S is %016llx\n", sw, name);
    return ret;
}
QUICK_HOOK_DEF(SwitchInput_hasOnNow, int32_t, void *a, wchar_t *b)
QUICK_HOOK_IMPL(SwitchInput_hasOnNow, int32_t, RETURN_NORMAL("%08x"), "%p::%S", a, b)
QUICK_HOOK_DEF(SwitchInput_hasOffNow, int32_t, void *a, wchar_t *b)
QUICK_HOOK_IMPL(SwitchInput_hasOffNow, int32_t, RETURN_NORMAL("%08x"), "%p::%S", a, b)

// undefined8 UsbIOUniqueOutput_set(undefined8 param_1,undefined param_2,undefined8 param_3,undefined4 param_4)
QUICK_HOOK_DEF(UsbIOUniqueOutput_set, void *, void *a, int8_t b, void *c, int32_t d)
QUICK_HOOK_IMPL(UsbIOUniqueOutput_set, void *, RETURN_NORMAL("%p"), "%p, %d, %p, %d", a, b, c, d)
QUICK_HOOK_DEF(Input_getPlayer, void *, int index)
QUICK_HOOK_IMPL(Input_getPlayer, void *, RETURN_NORMAL("%p"), "%d", index)
QUICK_HOOK_DEF(Input_getPlayerCount, int, void)
QUICK_HOOK_IMPL_NOARGS(Input_getPlayerCount, int, RETURN_NORMAL("%d"))
QUICK_HOOK_DEF(Input_getSystem, void *, void)
QUICK_HOOK_IMPL_NOARGS(Input_getSystem, void *, RETURN_NORMAL("%p"))

static struct hook_symbol_ex amdaemon_syms[] = {
    // QUICK_HOOK_SYM(UsbIOGeneralOutput_resetBits, 15),
    // QUICK_HOOK_SYM(UsbIOGeneralOutput_setBit, 15),
    // QUICK_HOOK_SYM(UsbIOGeneralOutput_setBits, 15),
    // QUICK_HOOK_SYM(UsbIOAnalogInput_getChannelCount, 14),
    // QUICK_HOOK_SYM(UsbIOAnalogInput_getValue, 14),
    QUICK_HOOK_SYM(UsbDevice_startReconnect, 16),
    // QUICK_HOOK_SYM_WITH_CALL(System_getBoardId, 14, 9, 19),
    // QUICK_HOOK_SYM(UsbIONode_getGeneralOutput, 14),
    // QUICK_HOOK_SYM(BoardIO_setLedState, 14),
    // QUICK_HOOK_SYM(AnalogInput_getDelta, 16),
    // QUICK_HOOK_SYM(AnalogInput_getValue, 16),
    // QUICK_HOOK_SYM(AnalogOutput_getCurrentValue, 16),
    // QUICK_HOOK_SYM(AnalogOutput_setValue, 16),
    QUICK_HOOK_SYM(UsbIO_getNode, 12),
    QUICK_HOOK_SYM_WITH_CALL(UsbIO_getNodeCount, 14, 9, 19),
    QUICK_HOOK_SYM(SwitchOutput_set, 15),
    QUICK_HOOK_SYM(SwitchInput_isOn, 14),
    // QUICK_HOOK_SYM(SwitchInput_hasOnNow, 14),
    // QUICK_HOOK_SYM(SwitchInput_hasOffNow, 14),
    QUICK_HOOK_SYM(UsbIOUniqueOutput_set, 15),
    QUICK_HOOK_SYM(Input_getPlayer, 14),
    QUICK_HOOK_SYM(Input_getPlayerCount, 14),
    QUICK_HOOK_SYM(Input_getSystem, 14),
};

// occupies 14 bytes after buf
void construct_long_jump(int8_t *buf, int64_t target)
{
    // Nikolay’s method
    int32_t lower_target = (int32_t)target;
    int32_t upper_target = (int32_t)(target >> 32);
    // push lower_target
    buf[0] = 0x68;
    *(int32_t *)(buf + 1) = lower_target;
    if (upper_target == 0)
    {
        // ret
        buf[5] = 0xC3;
        return;
    }
    // mov DWORD PTR [rsp+0x4], upper_target
    *(int32_t *)(buf + 5) = 0x042444C7;
    *(int32_t *)(buf + 9) = upper_target;
    // ret
    buf[13] = 0xC3;
}

// occupies 10 bytes before buf, and 10 bytes after buf
void construct_split_long_jump(int8_t *buf, int64_t target)
{
    // Nikolay’s method
    int32_t lower_target = (int32_t)target;
    int32_t upper_target = (int32_t)(target >> 32);
    int8_t *another_buf = buf - 10;
    // push lower_target
    buf[0] = 0x68;
    *(int32_t *)(buf + 1) = lower_target;
    if (upper_target == 0)
    {
        // ret
        buf[5] = 0xC3;
        return;
    }
    // jmp another_buf
    buf[5] = 0xE9;
    *(int32_t *)(buf + 6) = -20;

    // another_buf:
    // mov DWORD PTR [rsp+0x4], upper_target
    *(int32_t *)another_buf = 0x042444C7;
    *(int32_t *)(another_buf + 4) = upper_target;
    // ret
    another_buf[8] = 0xC3;
}

void amdaemon_hook_patch(HMODULE target)
{
    FARPROC addr;
    struct hook_symbol_ex *sym;
    DWORD old_protect, t;
    int copy_size, operation_size, move_offset;
    bool moving_needed;
    int32_t *call_dest;

    for (size_t i = 0; i < _countof(amdaemon_syms); i++)
    {
        sym = &amdaemon_syms[i];
        addr = GetProcAddress(target, sym->name);

        if (addr == NULL)
        {
            dprintf("Failed to find symbol %s\n", sym->name);
            continue;
        }
        if (sym->overwrite_size < 10)
        {
            dprintf("Symbol %s has an overwrite size less than 10\n", sym->name);
            continue;
        }

        copy_size = sym->overwrite_size;
        operation_size = sym->overwrite_size;
        moving_needed = sym->only_call >= 0 && sym->only_call < sym->overwrite_size;
        if (moving_needed)
        {
            copy_size = sym->only_call;
            move_offset = sym->overwrite_size - sym->only_call;
            operation_size = sym->function_size + move_offset;
        }

        // dprintf("Start patching %s, address: %p\n", sym->name, addr);

        VirtualProtect(addr, operation_size, PAGE_EXECUTE_READWRITE, &old_protect);

        // trampoline code:
        // first operation is the first operation of original function
        memcpy(sym->trampoline, addr, copy_size);
        // the next operation is to jump to the second operation of original function
        construct_long_jump(
            (int8_t *)sym->trampoline + copy_size,
            (int64_t)addr + sym->overwrite_size);
        // mark the trampoline as executable
        VirtualProtect(sym->trampoline, copy_size + 14, PAGE_EXECUTE_READWRITE, &t);
        // record the trampoline as the new address of the original function
        *sym->link = (void *)sym->trampoline;

        // original code: jump to the patch
        if (sym->overwrite_size < 14)
        {
            construct_split_long_jump((int8_t *)addr, (int64_t)sym->patch);
        }
        else
        {
            construct_long_jump((int8_t *)addr, (int64_t)sym->patch);
        }
        if (moving_needed)
        {
            // move the rest of the original function out of overwrite_size
            memcpy(
                (int8_t *)addr + sym->overwrite_size,
                (int8_t *)addr + sym->only_call,
                sym->function_size - sym->only_call);
            // and modify the call destination
            call_dest = (int32_t *)(addr + sym->overwrite_size + 1);
            *call_dest -= move_offset;
            uint8_t *a = addr + sym->overwrite_size + sym->function_size - sym->only_call - 1;
            dprintf(
                "%p %p %02x %02x %02x %02x\n",
                a,
                addr + sym->function_size,
                a[0],
                a[-1],
                a[-2],
                a[-3]);
        }

        VirtualProtect(addr, operation_size, old_protect, &old_protect);

        dprintf(
            "Patched %s, original address: %p, trampoline address: %p, patch address: %p\n",
            sym->name,
            addr,
            sym->trampoline,
            sym->patch);
    }
}

FARPROC amdeamon_hook_patch_one(FARPROC src, const char *name)
{
    for (size_t i = 0; i < _countof(amdaemon_syms); i++)
    {
        if (strcmp(amdaemon_syms[i].name, name) == 0)
        {
            *amdaemon_syms[i].link = src;
            return amdaemon_syms[i].patch;
        }
    }
    return NULL;
}