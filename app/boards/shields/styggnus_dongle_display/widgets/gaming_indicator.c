/*
 * Copyright (c) 2020 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#include "gaming_indicator.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <zmk/display.h>
#include <zmk/events/layer_state_changed.h>
#include <zmk/event_manager.h>
#include <zmk/endpoints.h>
#include <zmk/keymap.h>

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

struct layer_status_state {
    uint8_t index;
    const char *label;
};

#define LAYER_LETTERS 0
#define LAYER_GAMING 1
#define LAYER_NUMBERS 2
#define LAYER_NAV 3
#define LAYER_FROW 4

extern bool MurtuMMacroIsRecording();

static void set_layer_symbol(lv_obj_t *label, struct layer_status_state state) 
{
    if (MurtuMMacroIsRecording())
    {
        lv_label_set_text(label, "<REC>");
        return;
    }

    if (zmk_keymap_layer_state() & (BIT(LAYER_GAMING)))
    {
        lv_label_set_text(label, "Gaming");
        return;
    }
    
    char text[15] = {};

    switch (state.index)
    {
        case LAYER_LETTERS: snprintf(text, sizeof(text), ""); break;
        case LAYER_NUMBERS: snprintf(text, sizeof(text), "Num"); break;
        case LAYER_NAV: snprintf(text, sizeof(text), "Nav"); break;
        case LAYER_FROW: snprintf(text, sizeof(text), "F-Row"); break;
        default: snprintf(text, sizeof(text), "Unknown"); break;
    }

    lv_label_set_text(label, text);
}

static void layer_status_update_cb(struct layer_status_state state) {
    struct zmk_widget_gaming_indicator *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) { set_layer_symbol(widget->obj, state); }
}

static struct layer_status_state layer_status_get_state(const zmk_event_t *eh) {
    uint8_t index = zmk_keymap_highest_layer_active();
    return (struct layer_status_state) {
        .index = index,
        .label = zmk_keymap_layer_name(index)
    };
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_gaming_indicator, struct layer_status_state, layer_status_update_cb,
                            layer_status_get_state)

ZMK_SUBSCRIPTION(widget_gaming_indicator, zmk_layer_state_changed);

int zmk_widget_gaming_indicator_init(struct zmk_widget_gaming_indicator *widget, lv_obj_t *parent) {
    widget->obj = lv_label_create(parent);

    sys_slist_append(&widgets, &widget->node);

    widget_gaming_indicator_init();
    return 0;
}

lv_obj_t *zmk_widget_gaming_indicator_obj(struct zmk_widget_gaming_indicator *widget) {
    return widget->obj;
}