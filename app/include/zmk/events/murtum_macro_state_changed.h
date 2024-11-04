/*
 * Copyright (c) 2020 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <../../src/behaviors/behavior_murtum_macro.h>
#include <zephyr/kernel.h>
#include <zmk/event_manager.h>

struct zmk_murtum_macro_state_changed {
    bool recording;
    int num_entries;
    char macro[MM_MACRO_SIZE];
};

ZMK_EVENT_DECLARE(zmk_murtum_macro_state_changed);