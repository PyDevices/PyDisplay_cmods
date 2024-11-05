/*
 * SPDX-FileCopyrightText: 2024 Brad Barnett
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "esp_err.h"
#include "esp_log.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_task.h"
#include "esp_cpu.h"

#include "py/obj.h"
#include "py/runtime.h"
#include "py/mphal.h"
#include "py/mpstate.h"
#include "py/gc.h"
#include "py/stackctrl.h"
#include "py/nlr.h"
#include "py/objexcept.h"
#include "py/mpthread.h"
#include "shared/runtime/mpirq.h"
#include "shared/readline/readline.h"
#include "shared/runtime/pyexec.h"

#include "uart.h"
#include "usb.h"
#include "usb_serial_jtag.h"
#include "modmachine.h"
#include "modnetwork.h"
#include "mpthreadport.h"

#include "common.h"

// flag to indicate an esp_lcd_panel_io_tx_color operation is in progress
static volatile bool color_trans_active = false;

// cb_isr function taken directly from:
// https://github.com/lvgl/lv_binding_micropython/blob/master/driver/esp32/espidf.c
// Requires CONFIG_FREERTOS_INTERRUPT_BACKTRACE=n in sdkconfig
//
// Can't use mp_sched_schedule because lvgl won't yield to give micropython a chance to run
// Must run Micropython in ISR itself.
// Called in ISR context!
// removed mp_obj_t arg because we aren't passing an argument to the callback

static inline void cb_isr(mp_obj_t cb) {

    volatile uint32_t sp = (uint32_t)esp_cpu_get_sp(); // was (uint32_t)get_sp();

    // Calling micropython from ISR
    // See: https://github.com/micropython/micropython/issues/4895

    void *old_state = mp_thread_get_state();

    mp_state_thread_t ts; // local thread state for the ISR
    mp_thread_set_state(&ts);
    mp_stack_set_top((void*)sp); // need to include in root-pointer scan
    mp_stack_set_limit(512); // tune based on ISR thread stack size was (configIDLE_TASK_STACK_SIZE - 1024)
    mp_locals_set(mp_state_ctx.thread.dict_locals); // use main thread's locals
    mp_globals_set(mp_state_ctx.thread.dict_globals); // use main thread's globals

    mp_sched_lock(); // prevent VM from switching to another MicroPython thread
    gc_lock(); // prevent memory allocation

    nlr_buf_t nlr;
    if (nlr_push(&nlr) == 0) {
        mp_call_function_0(cb);  // changed to mp_call_function_0 and removed the arg
        nlr_pop();
    } else {
        // Uncaught exception in IRQ callback handler
        mp_obj_print_exception(&mp_plat_print, MP_OBJ_FROM_PTR(nlr.ret_val));  // changed to &mp_plat_print to fit this context
    }

    gc_unlock();
    mp_sched_unlock();

    mp_thread_set_state(old_state);
    mp_hal_wake_main_task_from_isr();
}

bool color_trans_done(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx) {
    bus_obj_t *self = (bus_obj_t *)user_ctx;
    if (mp_obj_is_callable(self->callback)) {
        cb_isr(self->callback);
    }
    color_trans_active = false;
    return false;
}

mp_obj_t send(size_t n_args, const mp_obj_t *args) {
    bus_obj_t *self = MP_OBJ_TO_PTR(args[0]);

    int cmd = -1;
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

    int cmd = -1;
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
