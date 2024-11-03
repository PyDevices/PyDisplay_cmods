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
#include "py/mphal.h"

#include "bus.h"
#include "spibus.h"
#include <string.h>


static void spibus_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    (void) kind;
    spibus_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_printf(print, "<SPIBus id=%d>", self->spi_host);
}

///
/// spi_bus - Configure a SPI bus.
///
/// Parameters:
///   - id: SPI host to use
///   - baudrate: Frequency of pixel clock
///   - polarity: Polarity
///   - phase: Phase
///   - bits: Bit-width of LCD parameter
///   - lsb_first: transmit LSB bit first
///   - sck: GPIO used for SCLK
///   - mosi: GPIO used for MOSI
///   - miso: GPIO used for MISO
///   - dc: GPIO used to select the D/C line, set this to -1 if the D/C line not controlled by manually pulling high/low GPIO
///   - cs: GPIO used for CS line
///

static mp_obj_t spibus_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args){
    enum {
        ARG_id,           // SPI host to use
        ARG_baudrate,            // Frequency of pixel clock
        ARG_polarity,           // Traditional SPI mode (0~3)
        ARG_phase,           // Traditional SPI mode (0~3)
        ARG_bits,     // Bit-width of LCD parameter
        ARG_lsb_first,          // transmit LSB bit first
        ARG_sck,        // GPIO used for SCLK
        ARG_mosi,        // GPIO used for MOSI
        ARG_miso,        // GPIO used for MISO
        ARG_dc,                 // GPIO used to select the D/C line, set this to -1 if the D/C line not controlled by manually pulling high/low GPIO
        ARG_cs,                 // GPIO used for CS line
    };

    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_id,               MP_ARG_INT  | MP_ARG_KW_ONLY, {.u_int = 2        } },
        { MP_QSTR_baudrate,         MP_ARG_INT  | MP_ARG_KW_ONLY, {.u_int = 24000000 } },
        { MP_QSTR_polarity,         MP_ARG_INT  | MP_ARG_KW_ONLY, {.u_int = 0        } },
        { MP_QSTR_phase,            MP_ARG_INT  | MP_ARG_KW_ONLY, {.u_int = 0       } },
        { MP_QSTR_bits,             MP_ARG_INT  | MP_ARG_KW_ONLY, {.u_int = 8        } },
        { MP_QSTR_lsb_first,        MP_ARG_BOOL | MP_ARG_KW_ONLY, {.u_bool = false   } },
        { MP_QSTR_sck,              MP_ARG_INT  | MP_ARG_KW_ONLY, {.u_int = -1       } },
        { MP_QSTR_mosi,             MP_ARG_INT  | MP_ARG_KW_ONLY, {.u_int = -1       } },
        { MP_QSTR_miso,             MP_ARG_INT  | MP_ARG_KW_ONLY, {.u_int = -1       } },
        { MP_QSTR_dc,               MP_ARG_INT  | MP_ARG_KW_ONLY, {.u_int = -1       } },
        { MP_QSTR_cs,               MP_ARG_INT  | MP_ARG_KW_ONLY, {.u_int = -1       } },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    spibus_obj_t *self = m_new_obj(spibus_obj_t);
    self->base.type = &spibus_type;
    self->spi_host = args[ARG_id].u_int;

    spi_bus_config_t buscfg = {
        .sclk_io_num = args[ARG_sck].u_int,
        .mosi_io_num = args[ARG_mosi].u_int,
        .miso_io_num = args[ARG_miso].u_int,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 0
    };
    ESP_ERROR_CHECK(spi_bus_initialize(self->spi_host, &buscfg, SPI_DMA_CH_AUTO));

    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = args[ARG_dc].u_int,
        .cs_gpio_num = args[ARG_cs].u_int,
        .pclk_hz = args[ARG_baudrate].u_int,
        .spi_mode = (args[ARG_polarity].u_int & 1) | ((args[ARG_phase].u_int & 1) << 1),
        .lcd_cmd_bits = args[ARG_bits].u_int,
        .lcd_param_bits = args[ARG_bits].u_int,
        .trans_queue_depth = 10,
        .on_color_trans_done = color_trans_done,
        .user_ctx = self,
        .flags.lsb_first = args[ARG_lsb_first].u_bool,
        .flags.dc_low_on_data = false,
        .flags.octal_mode = false,
    };
    self->io_handle = NULL;
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)self->spi_host, &io_config, &self->io_handle));

    return MP_OBJ_FROM_PTR(self);
}


static const mp_rom_map_elem_t spibus_locals_dict_table[] = {
    {MP_ROM_QSTR(MP_QSTR_send), MP_ROM_PTR(&send)},
    {MP_ROM_QSTR(MP_QSTR_send_color), MP_ROM_PTR(&send_color)},
    {MP_ROM_QSTR(MP_QSTR_register_callback), MP_ROM_PTR(&register_callback)},
    {MP_ROM_QSTR(MP_QSTR_swap_bytes), MP_ROM_PTR(&swap_bytes)},
};
static MP_DEFINE_CONST_DICT(spibus_locals_dict, spibus_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    spibus_type,
    MP_QSTR_SPI_BUS,
    MP_TYPE_FLAG_NONE,
    print, spibus_print,
    make_new, spibus_make_new,
    locals_dict, &spibus_locals_dict);


static const mp_map_elem_t spibus_module_globals_table[] = {
    {MP_ROM_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_spibus)},
    {MP_ROM_QSTR(MP_QSTR_SPIBus), (mp_obj_t)&spibus_type},
};
static MP_DEFINE_CONST_DICT(mp_module_spibus_globals, spibus_module_globals_table);

const mp_obj_module_t mp_module_spibus = {
    .base = {&mp_type_module},
    .globals = (mp_obj_dict_t *)&mp_module_spibus_globals,
};
MP_REGISTER_MODULE(MP_QSTR_spibus, mp_module_spibus);
