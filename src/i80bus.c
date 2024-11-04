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

#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "driver/gpio.h"

#include "py/obj.h"
#include "py/runtime.h"

#include "bus.h"
#include "i80bus.h"
#include <string.h>


/// i80bus
/// Configure a i8080 parallel bus.
///
/// Parameters:
///   - dc: data/command pin number
///   - cs: chip select pin number
///   - wr: write pin number
///   - data: tuple list of data pins
///   - freq: pixel clock frequency in Hz
///

static mp_obj_t i80bus_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args)
{
    enum {
        ARG_dc,
        ARG_cs,
        ARG_wr,
        ARG_data,
        ARG_freq,
    };

    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_dc,   MP_ARG_INT  | MP_ARG_REQUIRED                      },
        { MP_QSTR_cs,   MP_ARG_INT  | MP_ARG_REQUIRED                      },
        { MP_QSTR_wr,   MP_ARG_INT  | MP_ARG_REQUIRED                      },
        { MP_QSTR_data, MP_ARG_OBJ  | MP_ARG_REQUIRED                      },
        { MP_QSTR_freq, MP_ARG_INT  | MP_ARG_KW_ONLY, {.u_int = 10000000 } },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    bus_obj_t *self = m_new_obj(bus_obj_t);
    self->base.type = &i80bus_type;

    mp_obj_tuple_t *data_pins = MP_OBJ_TO_PTR(args[ARG_data].u_obj);
    if (data_pins->len > 24) {
        mp_raise_ValueError("data bus width must be <= 24");
    }

    int data_gpio_nums[24];
    for (size_t i = 0; i < data_pins->len; i++) {
        data_gpio_nums[i] = mp_obj_get_int(data_pins->items[i]);
    }
    for (size_t i = data_pins->len; i < 24; i++) {
        data_gpio_nums[i] = -1;
    }

    esp_lcd_i80_bus_config_t bus_config = {
        .dc_gpio_num = args[ARG_dc].u_int,
        .wr_gpio_num = args[ARG_wr].u_int,
        .clk_src = LCD_CLK_SRC_PLL160M, // same as default in IDF5 and 0 in the enum of IDF4.4
        .bus_width = data_pins->len,
        .max_transfer_bytes = 0,
    };
    memcpy(bus_config.data_gpio_nums, data_gpio_nums, sizeof(data_gpio_nums));

    esp_lcd_i80_bus_handle_t bus_handle = NULL;
    ESP_ERROR_CHECK(esp_lcd_new_i80_bus(&bus_config, &bus_handle));

    esp_lcd_panel_io_i80_config_t io_config = {
        .cs_gpio_num = args[ARG_cs].u_int,
        .pclk_hz = args[ARG_freq].u_int,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .trans_queue_depth = 10,
        .on_color_trans_done = color_trans_done,
        .user_ctx = self,
        .dc_levels = {
            .dc_data_level = 1,
            .dc_cmd_level = 0,
            .dc_dummy_level = 0,
            .dc_idle_level = 0,
        },
        .flags = {
            .cs_active_high = false,
            .reverse_color_bits = false,
            .swap_color_bytes = false,
            .pclk_active_neg = false,
            .pclk_idle_low = false,
        }
    };
    self->io_handle = NULL;
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i80(bus_handle, &io_config, &self->io_handle));

    return MP_OBJ_FROM_PTR(self);
}


MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(i80bus_send_obj, 1, 3, send);
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(i80bus_send_color_obj, 1, 3, send_color);
MP_DEFINE_CONST_FUN_OBJ_2(i80bus_register_callback_obj, register_callback);
MP_DEFINE_CONST_FUN_OBJ_2(i80bus_swap_bytes_obj, swap_bytes);

static const mp_rom_map_elem_t i80bus_locals_dict_table[] = {
    {MP_ROM_QSTR(MP_QSTR_send), MP_ROM_PTR(&i80bus_send_obj)},
    {MP_ROM_QSTR(MP_QSTR_send_color), MP_ROM_PTR(&i80bus_send_color_obj)},
    {MP_ROM_QSTR(MP_QSTR_register_callback), MP_ROM_PTR(&i80bus_register_callback_obj)},
    {MP_ROM_QSTR(MP_QSTR_swap_bytes), MP_ROM_PTR(&i80bus_swap_bytes_obj)},
};
static MP_DEFINE_CONST_DICT(i80bus_locals_dict, i80bus_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    i80bus_type,
    MP_QSTR_I80_BUS,
    MP_TYPE_FLAG_NONE,
    make_new, i80bus_make_new,
    locals_dict, &i80bus_locals_dict);


static const mp_map_elem_t i80bus_module_globals_table[] = {
    {MP_ROM_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_i80bus)},
    {MP_ROM_QSTR(MP_QSTR_I80Bus), (mp_obj_t)&i80bus_type},
};
static MP_DEFINE_CONST_DICT(mp_module_i80bus_globals, i80bus_module_globals_table);

const mp_obj_module_t mp_module_i80bus = {
    .base = {&mp_type_module},
    .globals = (mp_obj_dict_t *)&mp_module_i80bus_globals,
};
MP_REGISTER_MODULE(MP_QSTR_i80bus, mp_module_i80bus);
