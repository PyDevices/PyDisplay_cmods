/*
 * SPDX-FileCopyrightText: 2024 Brad Barnett
 *
 * SPDX-License-Identifier: MIT
 */


#include <stdio.h>
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_rgb.h"
#include "esp_lcd_panel_io.h"
#include "esp_err.h"
#include "esp_log.h"

#include "py/obj.h"
#include "py/runtime.h"
#include "py/mphal.h"


typedef struct _rgbframebuffer_obj_t {
    mp_obj_base_t base;                     // base class
    esp_lcd_panel_handle_t panel_handle;    // panel handle
    uint16_t width;                         // width of the framebuffer
    uint16_t height;                        // height of the framebuffer
    mp_buffer_info_t bufinfo;
} rgbframebuffer_obj_t;

extern const mp_obj_type_t rgbframebuffer_type;

static const char *TAG = "rgbframebuffer";

static mp_obj_t rgbframebuffer_refresh(mp_obj_t self_in, mp_obj_t buffer){
    rgbframebuffer_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(buffer, &bufinfo, MP_BUFFER_WRITE);
    void *buf = bufinfo.buf;
    esp_lcd_panel_draw_bitmap(self->panel_handle, 0, 0, self->width + 1, self->height + 1, buf);
    // call a done callback if desired
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(rgbframebuffer_refresh_obj, rgbframebuffer_refresh);

static mp_int_t rgbframebuffer_get_buffer(mp_obj_t self_in, mp_buffer_info_t *bufinfo, mp_uint_t flags) {
    rgbframebuffer_obj_t *self = MP_OBJ_TO_PTR(self_in);
    *bufinfo = self->bufinfo;
    bufinfo->typecode = 'H';
    return 0;
}

mp_obj_t rgbframebuffer_attr(mp_obj_t self_in, qstr attr, mp_obj_t *dest) {
    rgbframebuffer_obj_t *self = MP_OBJ_TO_PTR(self_in);
    if (dest[0] == MP_OBJ_NULL) {
        if (attr == MP_QSTR_width) {
            dest[0] = MP_OBJ_NEW_SMALL_INT(self->width);
        } else if (attr == MP_QSTR_height) {
            dest[0] = MP_OBJ_NEW_SMALL_INT(self->height);
        }
    }
    return mp_const_none;
}

static mp_obj_t rgbframebuffer_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    enum { ARG_de, ARG_vsync, ARG_hsync, ARG_dclk, ARG_red, ARG_green, ARG_blue, ARG_frequency, ARG_width, ARG_height, ARG_hsync_pulse_width, ARG_hsync_front_porch, ARG_hsync_back_porch,
    ARG_vsync_pulse_width, ARG_vsync_front_porch, ARG_vsync_back_porch, ARG_hsync_idle_low, ARG_vsync_idle_low,
    ARG_de_idle_high, ARG_pclk_active_high, ARG_pclk_idle_high};
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_de, MP_ARG_REQUIRED | MP_ARG_INT },
        { MP_QSTR_vsync, MP_ARG_REQUIRED | MP_ARG_INT },
        { MP_QSTR_hsync, MP_ARG_REQUIRED | MP_ARG_INT },
        { MP_QSTR_dclk, MP_ARG_REQUIRED | MP_ARG_INT },
        { MP_QSTR_red, MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_green, MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_blue, MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_frequency, MP_ARG_REQUIRED | MP_ARG_INT },
        { MP_QSTR_width, MP_ARG_REQUIRED | MP_ARG_INT },
        { MP_QSTR_height, MP_ARG_REQUIRED | MP_ARG_INT },
        { MP_QSTR_hsync_pulse_width, MP_ARG_REQUIRED | MP_ARG_INT },
        { MP_QSTR_hsync_front_porch, MP_ARG_REQUIRED | MP_ARG_INT },
        { MP_QSTR_hsync_back_porch, MP_ARG_REQUIRED | MP_ARG_INT },
        { MP_QSTR_vsync_pulse_width, MP_ARG_REQUIRED | MP_ARG_INT },
        { MP_QSTR_vsync_front_porch, MP_ARG_REQUIRED | MP_ARG_INT },
        { MP_QSTR_vsync_back_porch, MP_ARG_REQUIRED | MP_ARG_INT },
        { MP_QSTR_hsync_idle_low, MP_ARG_REQUIRED | MP_ARG_BOOL },
        { MP_QSTR_vsync_idle_low, MP_ARG_REQUIRED | MP_ARG_BOOL },
        { MP_QSTR_de_idle_high, MP_ARG_REQUIRED | MP_ARG_BOOL },
        { MP_QSTR_pclk_active_high, MP_ARG_REQUIRED | MP_ARG_BOOL },
        { MP_QSTR_pclk_idle_high, MP_ARG_REQUIRED | MP_ARG_BOOL },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    rgbframebuffer_obj_t *self = m_new_obj(rgbframebuffer_obj_t);
    self->base.type = &rgbframebuffer_type;
    self->width = args[ARG_width].u_int;
    self->height = args[ARG_height].u_int;
    
    ESP_LOGI(TAG, "Install RGB LCD panel driver");
    esp_lcd_panel_handle_t panel_handle = NULL;
    esp_lcd_rgb_panel_config_t panel_config = {
        .clk_src = LCD_CLK_SRC_DEFAULT,
        .timings = {
            .pclk_hz = args[ARG_frequency].u_int,
            .h_res = args[ARG_width].u_int,
            .v_res = args[ARG_height].u_int,
            .hsync_pulse_width = args[ARG_hsync_pulse_width].u_int,
            .hsync_front_porch = args[ARG_hsync_front_porch].u_int,
            .hsync_back_porch = args[ARG_hsync_back_porch].u_int,
            .vsync_pulse_width = args[ARG_vsync_pulse_width].u_int,
            .vsync_front_porch = args[ARG_vsync_front_porch].u_int,
            .vsync_back_porch = args[ARG_vsync_back_porch].u_int,
            .flags = {
                .hsync_idle_low = args[ARG_hsync_idle_low].u_bool,
                .vsync_idle_low = args[ARG_vsync_idle_low].u_bool,
                .de_idle_high = args[ARG_de_idle_high].u_bool,
                .pclk_active_neg = !args[ARG_pclk_active_high].u_bool,
                .pclk_idle_high = args[ARG_pclk_idle_high].u_bool,
            },
        },
        .data_width = 16, // RGB565 in parallel mode, thus 16bit in width
        .bits_per_pixel = 16,
        .num_fbs = 1,
#if CONFIG_EXAMPLE_USE_BOUNCE_BUFFER
        .bounce_buffer_size_px = 10 * args[ARG_width].u_int,
#endif
        .sram_trans_align = 64,
        .psram_trans_align = 64,
        .hsync_gpio_num = args[ARG_hsync].u_int,
        .vsync_gpio_num = args[ARG_vsync].u_int,
        .de_gpio_num = args[ARG_de].u_int,
        .pclk_gpio_num = args[ARG_dclk].u_int,
        .disp_gpio_num = -1,
        .flags = {
            .disp_active_low = false,
            .refresh_on_demand = false,
            .fb_in_psram = false,
            .double_fb = false,
            .no_fb = false,
            .bb_invalidate_cache = false,
        },
    };

    // Extract the pins from the tuples
    size_t pin_idx = 0;
    mp_obj_t *items;
    size_t len;

    mp_obj_get_array(args[ARG_blue].u_obj, &len, &items);
    for (size_t i = 0; i < len; i++) {
        panel_config.data_gpio_nums[pin_idx++] = mp_obj_get_int(items[i]);
    }

    mp_obj_get_array(args[ARG_green].u_obj, &len, &items);
    for (size_t i = 0; i < len; i++) {
        panel_config.data_gpio_nums[pin_idx++] = mp_obj_get_int(items[i]);
    }

    mp_obj_get_array(args[ARG_red].u_obj, &len, &items);
    for (size_t i = 0; i < len; i++) {
        panel_config.data_gpio_nums[pin_idx++] = mp_obj_get_int(items[i]);
    }

    ESP_ERROR_CHECK(esp_lcd_new_rgb_panel(&panel_config, &panel_handle));

    self->panel_handle = panel_handle;

    // ESP_LOGI(TAG, "Register event callbacks");
    // esp_lcd_rgb_panel_event_callbacks_t cbs = {
    //     .on_vsync = NULL,
    // };
    // ESP_ERROR_CHECK(esp_lcd_rgb_panel_register_event_callbacks(panel_handle, &cbs, NULL));

    ESP_LOGI(TAG, "Initialize RGB LCD panel");
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));

    void *buf = NULL;
#if 1
    ESP_ERROR_CHECK(esp_lcd_rgb_panel_get_frame_buffer(panel_handle, 1, &buf));
#else
    buf = heap_caps_malloc(EXAMPLE_LCD_H_RES * 100 * 2, MALLOC_CAP_SPIRAM);
    assert(buf);
#endif
    mp_get_buffer_raise(MP_OBJ_FROM_PTR(buf), &self->bufinfo, MP_BUFFER_WRITE);

    return self;
}



static const mp_rom_map_elem_t rgbframebuffer_locals_dict_table[] = {
    {MP_ROM_QSTR(MP_QSTR_refresh), MP_ROM_PTR(&rgbframebuffer_refresh_obj)},
};
static MP_DEFINE_CONST_DICT(rgbframebuffer_locals_dict, rgbframebuffer_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    rgbframebuffer_type,
    MP_QSTR_RGBFrameBuffer,
    MP_TYPE_FLAG_HAS_SPECIAL_ACCESSORS,
    make_new, rgbframebuffer_make_new,
    locals_dict, &rgbframebuffer_locals_dict,
    buffer, rgbframebuffer_get_buffer,
    attr, rgbframebuffer_attr);


static const mp_map_elem_t rgbframebuffer_module_globals_table[] = {
    {MP_ROM_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_rgbframebuffer)},
    {MP_ROM_QSTR(MP_QSTR_RGBFrameBuffer), (mp_obj_t)&rgbframebuffer_type},
};
static MP_DEFINE_CONST_DICT(mp_module_rgbframebuffer_globals, rgbframebuffer_module_globals_table);

const mp_obj_module_t mp_module_rgbframebuffer = {
    .base = {&mp_type_module},
    .globals = (mp_obj_dict_t *)&mp_module_rgbframebuffer_globals,
};
MP_REGISTER_MODULE(MP_QSTR_rgbframebuffer, mp_module_rgbframebuffer);
