/*
 * SPDX-FileCopyrightText: 2024 Brad Barnett
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef __BUS_H__
#define __BUS_H__

#include "py/obj.h"
#include "esp_lcd_panel_io.h"


typedef struct _bus_obj_t {
    mp_obj_base_t base;
    esp_lcd_panel_io_handle_t io_handle;
    bool trans_done;
    void (*tx_param)(esp_lcd_panel_io_handle_t, int, void *, int);
    void (*tx_color)(esp_lcd_panel_io_handle_t, int, void *, int);
} bus_obj_t;

bool color_trans_done(void *panel_io, void *edata, void *user_ctx);
mp_obj_t send(size_t n_args, const mp_obj_t *args);
mp_obj_t send_color(size_t n_args, const mp_obj_t *args);

#endif // __BUS_H__