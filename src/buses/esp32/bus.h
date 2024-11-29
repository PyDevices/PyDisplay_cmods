/*
 * SPDX-FileCopyrightText: 2024 Brad Barnett
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef __BUS_H__
#define __BUS_H__

#include "py/obj.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "driver/gpio.h"
#include "py/mphal.h"

typedef struct _bus_obj_t {
    mp_obj_base_t base;
    esp_lcd_panel_io_handle_t io_handle;
    bool trans_done;
    esp_err_t (*tx_param)(esp_lcd_panel_io_handle_t, int, const void *, size_t);
    esp_err_t (*tx_color)(esp_lcd_panel_io_handle_t, int, const void *, size_t);
} bus_obj_t;

#endif // __BUS_H__