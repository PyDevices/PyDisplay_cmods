/*
 * Modifications and additions Copyright (c) 2024 Brad Barnett, 2020-2023 Russ Hughes
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
#include <stdio.h>
#include <string.h>

#include "esp_err.h"
#include "esp_log.h"

#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "driver/gpio.h"

#include "py/obj.h"
#include "py/runtime.h"
#include "py/mphal.h"

#include "bus.h"


// flag to indicate an esp_lcd_panel_io_tx_color operation is in progress
static volatile bool color_trans_active = false;

bool color_trans_done(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx) {
    bus_obj_t *self = MP_OBJ_TO_PTR(user_ctx);
    mp_obj_t callback = (self->spi.base.type == &spibus_type) ? self->spi.callback : self->i80.callback;
    if (callback != mp_const_none) {
        mp_call_function_0(callback);
    }
    color_trans_active = false;
    return false;
}

mp_obj_t send(mp_obj_t self_in, mp_obj_t command, mp_obj_t data) {
    bus_obj_t *self = MP_OBJ_TO_PTR(self_in);
    esp_lcd_panel_io_handle_t io_handle = (self->spi.base.type == &spibus_type) ? self->spi.io_handle : self->i80.io_handle;

    if (command != mp_const_none) {
        int cmd = mp_obj_get_int(command);
        esp_lcd_panel_io_tx_param(io_handle, cmd, NULL, 0);
    }

    if (data != mp_const_none) {
        mp_buffer_info_t bufinfo;
        mp_get_buffer_raise(data, &bufinfo, MP_BUFFER_READ);
        esp_lcd_panel_io_tx_param(io_handle, 0, bufinfo.buf, bufinfo.len);
    }

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_3(send_obj, send);

mp_obj_t send_color(mp_obj_t self_in, mp_obj_t lcd_cmd, mp_obj_t color) {
    bus_obj_t *self = MP_OBJ_TO_PTR(self_in);
    esp_lcd_panel_io_handle_t io_handle = (self->spi.base.type == &spibus_type) ? self->spi.io_handle : self->i80.io_handle;
    int cmd = mp_obj_get_int(lcd_cmd);

    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(color, &bufinfo, MP_BUFFER_READ);
    void *buf = bufinfo.buf;

    while (color_trans_active) {
    }
    color_trans_active = true;

    esp_lcd_panel_io_tx_color(io_handle, cmd, buf, bufinfo.len);

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_3(trans_color_obj, send_color);

mp_obj_t register_callback(mp_obj_t self_in, mp_obj_t callback) {
    bus_obj_t *self = MP_OBJ_TO_PTR(self_in);
    // check to make sure callback is a micropython callable or None
    if (!mp_obj_is_callable(callback) && callback != mp_const_none) {
        mp_raise_ValueError("callback must be a callable object or None");
    }
    if (self->spi.base.type == &spibus_type) {
        self->spi.callback = callback;
    } else {
        self->i80.callback = callback;
    }
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(register_callback_obj, register_callback);
