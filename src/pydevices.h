#ifndef __PYDEVICES_H__
#define __PYDEVICES_H__

#include "py/obj.h"
#include "esp_lcd_panel_io.h"

bool color_trans_done(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx);
mp_obj_t pydevices_send(mp_obj_t self_in, mp_obj_t command, mp_obj_t data);
mp_obj_t pydevices_trans_color(mp_obj_t self_in, mp_obj_t lcd_cmd, mp_obj_t color);
mp_obj_t pydevices_register_callback(mp_obj_t self_in, mp_obj_t callback);

#endif // __PYDEVICES_H__