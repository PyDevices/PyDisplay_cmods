/*
 * SPDX-FileCopyrightText: 2024 Brad Barnett
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

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


bool color_trans_done(void *panel_io, void *edata, void *user_ctx) {
    bus_obj_t *self = (bus_obj_t *)user_ctx;
    self->trans_done = true;
    return false;
}

mp_obj_t send(size_t n_args, const mp_obj_t *args) {
    bus_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    int cmd = mp_obj_get_int(args[1]);
    void *buf = NULL;
    int len = 0;
    if (n_args > 2 && args[2] != mp_const_none) {
        mp_buffer_info_t bufinfo;
        mp_get_buffer_raise(args[2], &bufinfo, MP_BUFFER_READ);
        buf = bufinfo.buf;
        len = bufinfo.len;
    }

    self->tx_param(self->io_handle, cmd, buf, len);

    return mp_const_none;
}

mp_obj_t send_color(size_t n_args, const mp_obj_t *args) {    
    bus_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    int cmd = mp_obj_get_int(args[1]);
    void *buf = NULL;
    int len = 0;
    if (n_args > 2 && args[2] != mp_const_none) {
        mp_buffer_info_t bufinfo;
        mp_get_buffer_raise(args[2], &bufinfo, MP_BUFFER_READ);
        buf = bufinfo.buf;
        len = bufinfo.len;
    }

    self->trans_done = false;
    self->tx_color(self->io_handle, cmd, buf, len); // use the function pointer
    while (self->trans_done == false) {
        mp_handle_pending(true);
    }

    return mp_const_none;
}
