/*
 * Copyright (c) 2020 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <zephyr/kernel.h>
#include <zmk/event_manager.h>

struct zmk_murtum_macro_state_changed {
    struct behavior_murtum_macro_data* data;
};

ZMK_EVENT_DECLARE(zmk_murtum_macro_state_changed);