/*
 * SPDX-FileCopyrightText: 2024 Brad Barnett
 *
 * SPDX-License-Identifier: MIT
 */
/* 
ChatGPT 4o prompt:
I want to write a MicroPython USER_C_MODULE in C.  The module's name should be mod_byteswap and it should have a 
single function named func_byteswap.  func_byteswap should take a single argument that is a buffer of 16-bit data.
That buffer will most likely be a memoryview object.  func_byteswap should reverse the byte order of the array in
place, effectively changing its endianness.  Instead of treating the data as uint8_t, let's treat it as uin16_t
and swap the bytes using bit shifts.
 */

#include "py/obj.h"
#include "py/objarray.h"
#include "py/runtime.h"

// Function to swap bytes in a 16-bit buffer, in place
static mp_obj_t func_byteswap(mp_obj_t buf_obj) {
    // Ensure the input is a buffer (e.g., memoryview, bytearray, etc.)
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(buf_obj, &bufinfo, MP_BUFFER_RW);

    // Ensure the buffer is aligned with 16-bit elements
    if (bufinfo.len % 2 != 0) {
        mp_raise_ValueError(MP_ERROR_TEXT("Buffer length must be even (16-bit aligned)."));
    }

    // Get a pointer to the buffer
    uint16_t *buf = (uint16_t *)bufinfo.buf;
    size_t num_elements = bufinfo.len / 2;

    // Iterate over each 16-bit element and swap bytes using bit shifts
    for (size_t i = 0; i < num_elements; i++) {
        buf[i] = (buf[i] >> 8) | (buf[i] << 8);
    }

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(func_byteswap_obj, func_byteswap);

// Define the module's globals
static const mp_rom_map_elem_t mod_byteswap_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR_byteswap), MP_ROM_PTR(&func_byteswap_obj) },
};

static MP_DEFINE_CONST_DICT(mod_byteswap_globals, mod_byteswap_globals_table);

// Define the module
const mp_obj_module_t mod_byteswap_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&mod_byteswap_globals,
};

// Register the module to make it available in Python
MP_REGISTER_MODULE(MP_QSTR_byteswap, mod_byteswap_user_cmodule);
