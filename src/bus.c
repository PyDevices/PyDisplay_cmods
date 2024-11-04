/*
 * SPDX-FileCopyrightText: 2024 Brad Barnett
 *
 * SPDX-License-Identifier: MIT
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
    if (mp_obj_is_callable(self->callback)) {
        mp_call_function_0(self->callback);
    }
    color_trans_active = false;
    return false;
}

mp_obj_t send(size_t n_args, const mp_obj_t *args) {
    bus_obj_t *self = MP_OBJ_TO_PTR(args[0]);

    int cmd = 0;
    if (n_args > 1 && args[1] != mp_const_none) {
        cmd = mp_obj_get_int(args[1]);
    }

    void *buf = NULL;
    int len = 0;
    if (n_args > 2 && args[2] != mp_const_none) {
        mp_buffer_info_t bufinfo;
        mp_get_buffer_raise(args[2], &bufinfo, MP_BUFFER_READ);
        buf = bufinfo.buf;
        len = bufinfo.len;
    }
    esp_lcd_panel_io_tx_param(self->io_handle, cmd, buf, len);

    return mp_const_none;
}

mp_obj_t send_color(size_t n_args, const mp_obj_t *args) {
    bus_obj_t *self = MP_OBJ_TO_PTR(args[0]);

    int cmd = 0;
    if (args[1] != mp_const_none) {
        cmd = mp_obj_get_int(args[1]);
    }

    void *buf = NULL;
    int len = 0;
    if (args[2] != mp_const_none) {
        mp_buffer_info_t bufinfo;
        mp_get_buffer_raise(args[2], &bufinfo, MP_BUFFER_READ);
        buf = bufinfo.buf;
        len = bufinfo.len;
    }

    while (color_trans_active) {
    }
    color_trans_active = true;

    esp_lcd_panel_io_tx_color(self->io_handle, cmd, buf, len);

    return mp_const_none;
}

mp_obj_t register_callback(mp_obj_t self_in, mp_obj_t callback) {
    bus_obj_t *self = MP_OBJ_TO_PTR(self_in);
    // check to make sure callback is a micropython callable or None
    if (!mp_obj_is_callable(callback) && callback != mp_const_none) {
        mp_raise_ValueError("callback must be a callable object or None");
    }
    self->callback = callback;
    return mp_const_none;
}

mp_obj_t swap_bytes(mp_obj_t self_in, mp_obj_t data) {
    (void) self_in;
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(data, &bufinfo, MP_BUFFER_READ);
    uint16_t *buf = (uint16_t *)bufinfo.buf;
    size_t len = bufinfo.len / 2;
    for (size_t i = 0; i < len; i++) {
        buf[i] = (buf[i] >> 8) | (buf[i] << 8);
    }

    return mp_const_none;
}
