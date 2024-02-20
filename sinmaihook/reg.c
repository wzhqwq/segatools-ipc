/*
    This file is only used to simulate connected serial devices.

    Affected keys: HKLM\HARDWARE\DEVICEMAP\SERIALCOMM
    Virtualized values:
        \Device\Serial0 -> COM1
        \Device\Serial2 -> COM3
        \Device\Serial3 -> COM4
*/

#include <windows.h>

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

#include "hook/table.h"

#include "sinmaihook/reg.h"
#include "sinmaihook/dynamic-inject.h"

#include "util/dprintf.h"
#include "util/str.h"

/* API hooks */

static LSTATUS WINAPI hook_RegOpenKeyExW(
    HKEY key,
    const wchar_t *subkey,
    uint32_t flags,
    uint32_t access,
    HKEY *out);

static LSTATUS WINAPI hook_RegCloseKey(HKEY handle);

static LSTATUS WINAPI hook_RegQueryValueExW(
    HKEY handle,
    const wchar_t *name,
    void *reserved,
    uint32_t *type,
    void *bytes,
    uint32_t *nbytes);

static LSTATUS WINAPI hook_RegEnumValueW(
    HKEY handle,
    int32_t index,
    wchar_t *name,
    uint32_t *name_nbytes,
    uint32_t *reserved,
    uint32_t *type,
    void *bytes,
    uint32_t *nbytes);

static LSTATUS WINAPI hook_RegQueryInfoKeyW(
    HKEY hKey,
    wchar_t *class_bytes,
    uint32_t *class_nbytes,
    uint32_t *reserved,
    uint32_t *nsubkeys,
    uint32_t *max_subkey_nbytes,
    uint32_t *max_class_nbytes,
    uint32_t *nvalues,
    uint32_t *max_value_name_nbytes,
    uint32_t *max_value_nbytes,
    uint32_t *security_descriptor_nbytes,
    FILETIME *last_write_time);

/* Link pointers */

static LSTATUS(WINAPI *next_RegOpenKeyExW)(
    HKEY parent,
    const wchar_t *name,
    uint32_t flags,
    uint32_t access,
    HKEY *out);

static LSTATUS(WINAPI *next_RegCloseKey)(HKEY handle);

static LSTATUS(WINAPI *next_RegQueryValueExW)(
    HKEY handle,
    const wchar_t *name,
    void *reserved,
    uint32_t *type,
    void *bytes,
    uint32_t *nbytes);

static LSTATUS(WINAPI *next_RegEnumValueW)(
    HKEY handle,
    int32_t index,
    wchar_t *name,
    uint32_t *name_nbytes,
    uint32_t *reserved,
    uint32_t *type,
    void *bytes,
    uint32_t *nbytes);

static LSTATUS(WINAPI *next_RegQueryInfoKeyW)(
    HKEY hKey,
    wchar_t *class_bytes,
    uint32_t *class_nbytes,
    uint32_t *reserved,
    uint32_t *nsubkeys,
    uint32_t *max_subkey_nbytes,
    uint32_t *max_class_nbytes,
    uint32_t *nvalues,
    uint32_t *max_value_name_nbytes,
    uint32_t *max_value_nbytes,
    uint32_t *security_descriptor_nbytes,
    FILETIME *last_write_time);

static const struct hook_symbol reg_hook_syms[] = {
    {
        .name = "RegOpenKeyExW",
        .patch = hook_RegOpenKeyExW,
        .link = (void **)&next_RegOpenKeyExW,
    },
    {
        .name = "RegCloseKey",
        .patch = hook_RegCloseKey,
        .link = (void **)&next_RegCloseKey,
    },
    {
        .name = "RegQueryValueExW",
        .patch = hook_RegQueryValueExW,
        .link = (void **)&next_RegQueryValueExW,
    },
    {
        .name = "RegEnumValueW",
        .patch = hook_RegEnumValueW,
        .link = (void **)&next_RegEnumValueW,
    },
    {
        .name = "RegQueryInfoKeyW",
        .patch = hook_RegQueryInfoKeyW,
        .link = (void **)&next_RegQueryInfoKeyW,
    },
};

static CRITICAL_SECTION reg_hook_lock;
static HKEY serialComm_handle;
static struct reg_string_val serialComm_vals[] = {
    {
        .name = L"\\Device\\Serial0",
        .value = L"COM1",
    },
    {
        .name = L"\\Device\\Serial2",
        .value = L"COM3",
    },
    {
        .name = L"\\Device\\Serial3",
        .value = L"COM4",
    },
};
static size_t serialComm_nvals = _countof(serialComm_vals);
static wchar_t *serialComm_subkey = L"HARDWARE\\DEVICEMAP\\SERIALCOMM";
static HKEY serialComm_root = HKEY_LOCAL_MACHINE;

void reg_hook_init(void)
{
    InitializeCriticalSection(&reg_hook_lock);

    dyn_inject_push_syms(reg_hook_syms, _countof(reg_hook_syms));
}

static LSTATUS WINAPI hook_RegOpenKeyExW(
    HKEY key,
    const wchar_t *subkey,
    uint32_t flags,
    uint32_t access,
    HKEY *out)
{
    LSTATUS result;

    if (out == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

    result = next_RegOpenKeyExW(key, subkey, flags, access, out);

    EnterCriticalSection(&reg_hook_lock);
    if (result == ERROR_SUCCESS && key == serialComm_root && wcscmp(subkey, serialComm_subkey) == 0)
    {
        if (serialComm_handle == NULL)
        {
            serialComm_handle = *out;
        }
    }
    LeaveCriticalSection(&reg_hook_lock);

    return result;
}

static LSTATUS WINAPI hook_RegCloseKey(HKEY handle)
{
    EnterCriticalSection(&reg_hook_lock);
    if (handle == serialComm_handle)
    {
        serialComm_handle = NULL;
    }
    LeaveCriticalSection(&reg_hook_lock);

    return next_RegCloseKey(handle);
}

static LSTATUS WINAPI hook_RegQueryValueExW(
    HKEY handle,
    const wchar_t *name,
    void *reserved,
    uint32_t *type,
    void *bytes,
    uint32_t *nbytes)
{
    LSTATUS result = ERROR_SUCCESS;

    assert(name != NULL);

    EnterCriticalSection(&reg_hook_lock);
    if (handle == serialComm_handle)
    {
        if (type != NULL)
            *type = REG_SZ;

        if (bytes != NULL && nbytes == NULL)
        {
            result = ERROR_INVALID_PARAMETER;
            goto RegQueryValueExW_pass;
        }

        if (nbytes != NULL)
        {
            for (size_t i = 0; i < serialComm_nvals; i++)
            {
                if (wcscmp(name, serialComm_vals[i].name) == 0)
                {
                    size_t nbytes_needed = (wcslen(serialComm_vals[i].value) + 1) * sizeof(wchar_t);

                    if (bytes != NULL)
                    {
                        if (*nbytes < nbytes_needed)
                        {
                            result = ERROR_MORE_DATA;
                            goto RegQueryValueExW_pass;
                        }
                        memcpy(bytes, serialComm_vals[i].value, nbytes_needed);
                        // dprintf("Read serial comm value: %S -> %S\n", name, serialComm_vals[i].value);
                    }

                    *nbytes = nbytes_needed;
                    goto RegQueryValueExW_pass;
                }
            }
            result = ERROR_FILE_NOT_FOUND;
            goto RegQueryValueExW_pass;
        }
        goto RegQueryValueExW_pass;
    }
    result = next_RegQueryValueExW(handle, name, reserved, type, bytes, nbytes);

RegQueryValueExW_pass:
    LeaveCriticalSection(&reg_hook_lock);
    return result;
}

static LSTATUS WINAPI hook_RegEnumValueW(
    HKEY handle,
    int32_t index,
    wchar_t *name,
    uint32_t *name_nbytes,
    uint32_t *reserved,
    uint32_t *type,
    void *bytes,
    uint32_t *nbytes)
{
    LSTATUS result = ERROR_SUCCESS;

    EnterCriticalSection(&reg_hook_lock);
    if (handle == serialComm_handle)
    {
        if (index < 0 || index >= serialComm_nvals)
        {
            result = ERROR_NO_MORE_ITEMS;
            goto RegEnumValueW_pass;
        }

        if (type != NULL)
            *type = REG_SZ;

        if (name != NULL && name_nbytes == NULL)
        {
            result = ERROR_INVALID_PARAMETER;
            goto RegEnumValueW_pass;
        }
        if (bytes != NULL && nbytes == NULL)
        {
            result = ERROR_INVALID_PARAMETER;
            goto RegEnumValueW_pass;
        }

        if (name_nbytes != NULL)
        {
            size_t name_nbytes_needed = (wcslen(serialComm_vals[index].name) + 1) * sizeof(wchar_t);

            if (name != NULL)
            {
                if (*name_nbytes < name_nbytes_needed)
                {
                    result = ERROR_MORE_DATA;
                    goto RegEnumValueW_pass;
                }
                memcpy(name, serialComm_vals[index].name, name_nbytes_needed);
                // dprintf("Enumerate serial comm name: %S\n", serialComm_vals[index].name);
            }
            *name_nbytes = name_nbytes_needed;
        }
        if (nbytes != NULL)
        {
            size_t nbytes_needed = (wcslen(serialComm_vals[index].value) + 1) * sizeof(wchar_t);

            if (bytes != NULL)
            {
                if (*nbytes < nbytes_needed)
                {
                    result = ERROR_MORE_DATA;
                    goto RegEnumValueW_pass;
                }
                memcpy(bytes, serialComm_vals[index].value, nbytes_needed);
                // dprintf("Enumerate serial comm value: %S -> %S\n", serialComm_vals[index].name, serialComm_vals[index].value);
            }
            *nbytes = nbytes_needed;
        }
        goto RegEnumValueW_pass;
    }
    result = next_RegEnumValueW(
        handle,
        index,
        name,
        name_nbytes,
        reserved,
        type,
        bytes,
        nbytes);

RegEnumValueW_pass:
    LeaveCriticalSection(&reg_hook_lock);
    return result;
}

static LSTATUS WINAPI hook_RegQueryInfoKeyW(
    HKEY hKey,
    wchar_t *class_bytes,
    uint32_t *class_nbytes,
    uint32_t *reserved,
    uint32_t *nsubkeys,
    uint32_t *max_subkey_nbytes,
    uint32_t *max_class_nbytes,
    uint32_t *nvalues,
    uint32_t *max_value_name_nbytes,
    uint32_t *max_value_nbytes,
    uint32_t *security_descriptor_nbytes,
    FILETIME *last_write_time)
{
    LSTATUS result;

    EnterCriticalSection(&reg_hook_lock);
    if (hKey == serialComm_handle)
    {
        if (nsubkeys != NULL)
            *nsubkeys = 0;

        if (max_subkey_nbytes != NULL)
            *max_subkey_nbytes = 0;

        if (max_class_nbytes != NULL)
            *max_class_nbytes = 0;

        if (nvalues != NULL)
            *nvalues = serialComm_nvals;

        if (max_value_name_nbytes != NULL)
        {
            size_t max_nbytes = 0;
            for (size_t i = 0; i < serialComm_nvals; i++)
            {
                size_t nbytes = (wcslen(serialComm_vals[i].name) + 1) * sizeof(wchar_t);
                if (nbytes > max_nbytes)
                    max_nbytes = nbytes;
            }
            *max_value_name_nbytes = max_nbytes;
        }

        if (max_value_nbytes != NULL)
        {
            size_t max_nbytes = 0;
            for (size_t i = 0; i < serialComm_nvals; i++)
            {
                size_t nbytes = (wcslen(serialComm_vals[i].value) + 1) * sizeof(wchar_t);
                if (nbytes > max_nbytes)
                    max_nbytes = nbytes;
            }
            *max_value_nbytes = max_nbytes;
        }

        result = ERROR_SUCCESS;
        goto RegQueryInfoKeyW_pass;
    }
    result = next_RegQueryInfoKeyW(
        hKey,
        class_bytes,
        class_nbytes,
        reserved,
        nsubkeys,
        max_subkey_nbytes,
        max_class_nbytes,
        nvalues,
        max_value_name_nbytes,
        max_value_nbytes,
        security_descriptor_nbytes,
        last_write_time);

RegQueryInfoKeyW_pass:
    LeaveCriticalSection(&reg_hook_lock);
    return result;
}
