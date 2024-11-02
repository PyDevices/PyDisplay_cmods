/*
 * Modifications and additions Copyright (c) 2020-2023 Russ Hughes
 *
 * This file licensed under the MIT License and incorporates work covered by
 * the following copyright and permission notice:
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ivan Belokobylskiy
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "esp_err.h"
#include "esp_log.h"

#include "esp_lcd_panel_commands.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "soc/soc_caps.h"
#include "driver/gpio.h"
#include "driver/dedic_gpio.h"

#include "py/obj.h"
#include "py/objstr.h"
#include "py/objmodule.h"
#include "py/runtime.h"
#include "py/builtin.h"
#include "py/mphal.h"

// Fix for MicroPython > 1.21 https://github.com/ricksorensen
#if MICROPY_VERSION_MAJOR >= 1 && MICROPY_VERSION_MINOR > 21
#include "extmod/modmachine.h"
#else
#include "extmod/machine_spi.h"
#endif

#include "pydevices.h"
#include "i80bus.h"
#include "spibus.h"

#define TAG "PYDEVICES"


//
// flag to indicate an esp_lcd_panel_io_tx_color operation is in progress
//

static volatile bool color_trans_active = false;

bool color_trans_done(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx) {
    pydevices_spi_bus_obj_t *self = MP_OBJ_TO_PTR(user_ctx);
    if (self->callback != mp_const_none) {
        mp_call_function_0(self->callback);
    }
    color_trans_active = false;
    return false;
}

mp_obj_t pydevices_send(mp_obj_t self_in, mp_obj_t command, mp_obj_t data) {
    pydevices_spi_bus_obj_t *self = MP_OBJ_TO_PTR(self_in);

    if (command != mp_const_none) {
        int cmd = mp_obj_get_int(command);
        // Send command using ESP-IDF functions
        esp_lcd_panel_io_tx_param(self->io_handle, cmd, NULL, 0);
    }

    if (data != mp_const_none) {
        mp_buffer_info_t bufinfo;
        mp_get_buffer_raise(data, &bufinfo, MP_BUFFER_READ);
        // Send data using ESP-IDF functions
        esp_lcd_panel_io_tx_param(self->io_handle, 0, bufinfo.buf, bufinfo.len);
    }

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_3(pydevices_send_obj, pydevices_send);

mp_obj_t pydevices_trans_color(mp_obj_t self_in, mp_obj_t lcd_cmd, mp_obj_t color) {
    pydevices_spi_bus_obj_t *self = MP_OBJ_TO_PTR(self_in);
    int cmd = mp_obj_get_int(lcd_cmd);

    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(color, &bufinfo, MP_BUFFER_READ);
    void *buf = bufinfo.buf;

    while (color_trans_active) {
    }
    color_trans_active = true;

    esp_lcd_panel_io_tx_color(self->io_handle, cmd, buf, bufinfo.len);

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_3(pydevices_trans_color_obj, pydevices_trans_color);

mp_obj_t pydevices_register_callback(mp_obj_t self_in, mp_obj_t callback) {
    pydevices_spi_bus_obj_t *self = MP_OBJ_TO_PTR(self_in);
    // check to make sure callback is a micropython callable or None
    if (!mp_obj_is_callable(callback) && callback != mp_const_none) {
        mp_raise_ValueError("callback must be a callable object or None");
    }
    self->callback = callback;
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(pydevices_register_callback_obj, pydevices_register_callback);

static const mp_map_elem_t pydevices_module_globals_table[] = {
    {MP_ROM_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_pydevices)},
    {MP_ROM_QSTR(MP_QSTR_I80_BUS), (mp_obj_t)&pydevices_i80_bus_type},
    {MP_ROM_QSTR(MP_QSTR_SPIBus), (mp_obj_t)&pydevices_spi_bus_type},
};

static MP_DEFINE_CONST_DICT(mp_module_pydevices_globals, pydevices_module_globals_table);

const mp_obj_module_t mp_module_pydevices = {
    .base = {&mp_type_module},
    .globals = (mp_obj_dict_t *)&mp_module_pydevices_globals,
};

// use the following for older versions of MicroPython

#if MICROPY_VERSION >= 0x011300                     // MicroPython 1.19 or later
MP_REGISTER_MODULE(MP_QSTR_pydevices, mp_module_pydevices);
#else
MP_REGISTER_MODULE(MP_QSTR_pydevices, mp_module_pydevices, MODULE_ESPLCD_ENABLE);
#endif
