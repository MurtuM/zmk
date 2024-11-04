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
#include <zmk/events/murtum_macro_state_changed.h>
#include <zmk/event_manager.h>
#include <zmk/endpoints.h>
#include <zmk/keymap.h>

#include <inttypes.h>

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

// extern bool GetMurtuMMacroStatus(bool*, int*, uint32_t*);

int num_to_hide = 0;
lv_obj_t* to_hide[16];

static void HideStuff(bool Hide)
{
    for (int i = 0; i < num_to_hide; i++)
    {
        Hide ? lv_obj_add_flag(to_hide[i], LV_OBJ_FLAG_HIDDEN) : lv_obj_clear_flag(to_hide[i], LV_OBJ_FLAG_HIDDEN);
    }
}

void zmk_widget_murtum_macro_status_add_to_hide(lv_obj_t* obj)
{
    to_hide[num_to_hide++] = obj;
}

static void update_status(lv_obj_t *label, struct zmk_murtum_macro_state_changed state) 
{
    char text[MM_MACRO_SIZE] = {};

    if (state.recording)
    {
        HideStuff(true);
        
        // sprintf(text, "R(%i)", state.num_entries);
        int ti = 0;
        for (int i = 0; i < state.num_entries && i < 100; i++)
        {
            text[ti++] = state.macro[i];
            if (i > 0 && (i % 13 == 0))
                text[ti++] = '\n';
        }

        text[ti] = '\0';
    }
    else 
    {
        HideStuff(false);

        // sprintf(text, "idle (%i)", state.num_entries);
        // strncat(text, "n", 1);
    }

    lv_label_set_text(label, text);
}

static void murtum_macro_status_update_cb(struct zmk_murtum_macro_state_changed state) {
    struct zmk_widget_murtum_macro_status *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) { update_status(widget->obj, state); }
}

static struct zmk_murtum_macro_state_changed as_mm_state(const zmk_event_t *eh) 
{
    const struct zmk_murtum_macro_state_changed* mmsc = as_zmk_murtum_macro_state_changed(eh);
    if (mmsc == NULL)
    {
        return (struct zmk_murtum_macro_state_changed){.recording = false, .num_entries = 0};
    }

    return *mmsc;
}

ZMK_DISPLAY_WIDGET_LISTENER(mm_sc_listener, struct zmk_murtum_macro_state_changed, 
    murtum_macro_status_update_cb, as_mm_state)
ZMK_SUBSCRIPTION(mm_sc_listener, zmk_murtum_macro_state_changed);

int zmk_widget_murtum_macro_status_init(struct zmk_widget_murtum_macro_status *widget, lv_obj_t *parent) {
    widget->obj = lv_label_create(parent);

    sys_slist_append(&widgets, &widget->node);

    mm_sc_listener_init();

    return 0;
}