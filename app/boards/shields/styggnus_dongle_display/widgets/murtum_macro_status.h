/*
 * Copyright (c) 2024 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

 #pragma once

#include <lvgl.h>
#include <zephyr/kernel.h>

struct zmk_widget_murtum_macro_status {
    sys_snode_t node;
    lv_obj_t *obj;
};

int zmk_widget_murtum_macro_status_init(struct zmk_widget_murtum_macro_status *widget, lv_obj_t *parent);
void zmk_widget_murtum_macro_status_add_to_hide(lv_obj_t* obj);
