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

#ifndef __RGBFRAMEBUFFER_H__
#define __RGBFRAMEBUFFER_H__

#include "py/obj.h"
#include "esp_lcd_panel_io.h"


typedef struct _rgbframebuffer_obj_t {
    mp_obj_base_t base;                     // base class
    esp_lcd_panel_handle_t panel_handle;    // panel handle
    uint16_t width;                         // width of the framebuffer
    uint16_t height;                        // height of the framebuffer
    mp_buffer_info_t bufinfo;
} rgbframebuffer_obj_t;

extern const mp_obj_type_t rgbframebuffer_type;

#endif /* __RGBFRAMEBUFFER_H__ */
