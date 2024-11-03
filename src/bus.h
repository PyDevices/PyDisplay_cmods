/*
 * Copyright (c) 2024 Brad Barnett
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

#ifndef __BUS_H__
#define __BUS_H__

#include "py/obj.h"
#include "esp_lcd_panel_io.h"
#include "spibus.h"
#include "i80bus.h"


typedef union _bus_obj_t {
    spibus_obj_t spi;
    i80bus_obj_t i80;
} bus_obj_t;

bool color_trans_done(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx);
mp_obj_t send(mp_obj_t self_in, mp_obj_t command, mp_obj_t data);
mp_obj_t send_color(mp_obj_t self_in, mp_obj_t lcd_cmd, mp_obj_t color);
mp_obj_t register_callback(mp_obj_t self_in, mp_obj_t callback);

#endif // __BUS_H__