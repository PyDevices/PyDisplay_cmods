/*
 * SPDX-FileCopyrightText: 2024 Brad Barnett
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef __COMMON_H__
#define __COMMON_H__

#include "py/runtime.h"

#ifdef ESP_IDF_VERSION
#include "../esp32/bus.h"
#endif

bool color_trans_done(void *panel_io, void *edata, void *user_ctx);
mp_obj_t send(size_t n_args, const mp_obj_t *args);
mp_obj_t send_color(size_t n_args, const mp_obj_t *args);

#endif // __COMMON_H__