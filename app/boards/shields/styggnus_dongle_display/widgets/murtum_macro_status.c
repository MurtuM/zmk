/*
 * Copyright (c) 2020 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#include "murtum_macro_status.h"

#include <../src/behaviors/behavior_murtum_macro.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <zmk/display.h>
#include <zmk/events/keycode_state_changed.h>
#include <zmk/event_manager.h>
#include <zmk/endpoints.h>
#include <zmk/keymap.h>

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

struct mms_state {
    bool recording;
    int num_entries;
    uint32_t macro[MM_MACRO_SIZE];
};

extern bool GetMurtuMMacroStatus(bool*, int*, uint32_t*);

static void update_status(lv_obj_t *label, struct mms_state state) 
{
    // struct mms_state state;
    // state.recording = false;
    // state.num_entries = 0;
    
    // GetMurtuMMacroStatus(&state.recording, &state.num_entries, (uint32_t*)&state.macro);

    char text[MM_MACRO_SIZE] = {};

    if (state.recording)
    {
        sprintf(text, "r(%i)", state.num_entries);

        // for (int i = 0; i < state.num_entries; i++)   
        //     sprintf(text, "%i", state.macro[i]);
    }
    else 
    {
        sprintf(text, "(%i)", state.num_entries);
        // strncat(text, "n", 1);
    }

    lv_label_set_text(label, text);
}

static void murtum_macro_status_update_cb(struct mms_state state) {
    struct zmk_widget_murtum_macro_status *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) { update_status(widget->obj, state); }
}


static struct mms_state murtum_macro_status_get_state(const zmk_event_t *eh) 
{
    struct mms_state result;
    result.recording = false;
    result.num_entries = 0;
    
    GetMurtuMMacroStatus(&result.recording, &result.num_entries, (uint32_t*)&result.macro);

    return result;
}

ZMK_DISPLAY_WIDGET_LISTENER(mmkc_listener, struct mms_state, murtum_macro_status_update_cb,
                            murtum_macro_status_get_state)

ZMK_SUBSCRIPTION(mmkc_listener, zmk_keycode_state_changed);

int zmk_widget_murtum_macro_status_init(struct zmk_widget_murtum_macro_status *widget, lv_obj_t *parent) {
    widget->obj = lv_label_create(parent);

    sys_slist_append(&widgets, &widget->node);

    mmkc_listener_init();

    return 0;
}