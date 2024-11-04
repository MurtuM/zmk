#define DT_DRV_COMPAT zmk_behavior_murtum_macro

#include "behavior_murtum_macro.h"

#include <zephyr/device.h>
#include <drivers/behavior.h>
#include <zephyr/logging/log.h>
#include <zmk/behavior.h>
#include <zmk/hid.h>

#include <zmk/event_manager.h>
#include <zmk/events/keycode_state_changed.h>
#include <zmk/events/murtum_macro_state_changed.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)

static const struct behavior_parameter_value_metadata param_values[] = {
    {
        .display_name = "Mode",
        .type = BEHAVIOR_PARAMETER_VALUE_TYPE_VALUE,
    },
};

static const struct behavior_parameter_metadata_set param_metadata_set[] = {{
    .param1_values = param_values,
    .param1_values_len = ARRAY_SIZE(param_values),
}};

static const struct behavior_parameter_metadata metadata = {
    .sets_len = ARRAY_SIZE(param_metadata_set),
    .sets = param_metadata_set,
};

#endif

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

struct behavior_murtum_macro_config {
    uint8_t index;
    uint8_t usage_pages_count;
    uint16_t usage_pages[];
};

struct behavior_murtum_macro_data {
    struct zmk_keycode_state_changed macro[MM_MACRO_SIZE];
    bool restart_cursor;
    bool recording;
    int cursor;
};

static char to_char(uint32_t Keycode)
{
    switch(Keycode)
    {
        case 44: return ' ';
        case 41: return 'e';
        case 39: return '0';
        default: 
            if (Keycode >= 30 && Keycode < 39)
                return Keycode + 19;
        return Keycode + 61;
    }
}

static void RaiseMMEvent(struct behavior_murtum_macro_data* data)
{
    struct zmk_murtum_macro_state_changed sc;
    sc.recording = data->recording;
    sc.num_entries = 0;

    for (int i = 0; i < data->cursor; i++)
    {
        struct zmk_keycode_state_changed* kp = &data->macro[i];

        if (!kp->state)
            continue;
        
        sc.macro[sc.num_entries++] = to_char(kp->keycode);
    }

    LOG_DBG("murtum_macro RaiseMMEvent num_entries: %d", sc.num_entries);

    raise_zmk_murtum_macro_state_changed(sc);
}

static int on_murtum_macro_binding_pressed(struct zmk_behavior_binding *binding,
                                         struct zmk_behavior_binding_event event) 
{
    const struct device *dev = zmk_behavior_get_binding(binding->behavior_dev);
    struct behavior_murtum_macro_data *data = dev->data;

    LOG_DBG("murtum_macro (%d) param: %d cursor: %d", event.position, binding->param1, data->cursor);

    if (binding->param1 == MM_RECORD)
    {
        data->recording = !data->recording;
        data->restart_cursor = true;

        RaiseMMEvent(data);

        return ZMK_BEHAVIOR_OPAQUE;
    }
    
    if (binding->param1 == MM_PLAYBACK && data->cursor > 0)
    {
        data->recording = false;
        
        RaiseMMEvent(data);

        uint64_t t = k_uptime_get();
        for (int i = 0; i < data->cursor; i++)
        {
            struct zmk_keycode_state_changed* kp = &data->macro[i];

            kp->timestamp = t;
            t += 10;

            if (kp->state)
            {
                LOG_DBG("MM DOWN: up: %d keycode: %d imod: %hd emod: %hd time: %lld", kp->usage_page, kp->keycode, kp->implicit_modifiers, kp->explicit_modifiers, kp->timestamp);
            }
            else
            {
                LOG_DBG("MM UP: up: %d keycode: %d imod: %hd emod: %hd time: %lld", kp->usage_page, kp->keycode, kp->implicit_modifiers, kp->explicit_modifiers, kp->timestamp);
            }

            raise_zmk_keycode_state_changed(*kp);
        }

        return ZMK_BEHAVIOR_OPAQUE;
    }

    return ZMK_BEHAVIOR_OPAQUE;
}

static int on_murtum_macro_binding_released(struct zmk_behavior_binding *binding,
                                          struct zmk_behavior_binding_event event) 
{
    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api behavior_murtum_macro_driver_api = {
    .binding_pressed = on_murtum_macro_binding_pressed,
    .binding_released = on_murtum_macro_binding_released,
#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)
    .get_parameter_metadata = &metadata,
#endif // IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)
};

static int murtum_macro_keycode_state_changed_listener(const zmk_event_t *eh);

ZMK_LISTENER(behavior_murtum_macro, murtum_macro_keycode_state_changed_listener);
ZMK_SUBSCRIPTION(behavior_murtum_macro, zmk_keycode_state_changed);

static const struct device *devs[DT_NUM_INST_STATUS_OKAY(DT_DRV_COMPAT)];

static int murtum_macro_keycode_state_changed_listener(const zmk_event_t *eh) {
    struct zmk_keycode_state_changed *ev = as_zmk_keycode_state_changed(eh);
    // if (ev == NULL || !ev->state) {
    //     return ZMK_EV_EVENT_BUBBLE;
    // }

    if (ev == NULL)
    {
        return ZMK_EV_EVENT_BUBBLE;
    }

    for (int i = 0; i < DT_NUM_INST_STATUS_OKAY(DT_DRV_COMPAT); i++) 
    {
        const struct device *dev = devs[i];
        if (dev == NULL) {
            continue;
        }

        struct behavior_murtum_macro_data *data = dev->data;
        const struct behavior_murtum_macro_config *config = dev->config;

        if (!data->recording)
            continue;

        if (data->restart_cursor)
            data->cursor = 0;
        data->restart_cursor = false;

        if (data->cursor >= MM_MACRO_SIZE)
            continue;

        for (int u = 0; u < config->usage_pages_count; u++) 
        {
            if (config->usage_pages[u] == ev->usage_page) 
            {
                memcpy(&data->macro[data->cursor], ev, sizeof(struct zmk_keycode_state_changed));
                // data->macro[data->cursor].implicit_modifiers |= zmk_hid_get_explicit_mods();
                data->cursor++;
                RaiseMMEvent(data);
                break;
            }
        }
        
    }

    return ZMK_EV_EVENT_BUBBLE;
}

// bool MurtuMMacroIsRecording()
// {
//     if (DT_NUM_INST_STATUS_OKAY(DT_DRV_COMPAT) == 0 || devs[0] == NULL)
//         return false;

//     const struct behavior_murtum_macro_data *data = devs[0]->data;

//     return data->recording;
// }

// bool GetMurtuMMacroStatus(bool* recording, int* num_entries, uint32_t* macro)
// {
//     if (DT_NUM_INST_STATUS_OKAY(DT_DRV_COMPAT) == 0 || devs[0] == NULL)
//         return false;

//     const struct behavior_murtum_macro_data *data = devs[0]->data;

//     *recording = data->recording;

//     int reported_keys = 0;
//     for (int i = 0; i < data->cursor; i++)
//     {
//         if (data->macro[i].state)
//             macro[reported_keys++] = data->macro[i].keycode;
//     }

//     *num_entries = reported_keys;

//     return true;
// }

static int behavior_murtum_macro_init(const struct device *dev) {
    const struct behavior_murtum_macro_config *config = dev->config;
    devs[config->index] = dev;
    
    struct behavior_murtum_macro_data *data = dev->data;
    data->recording = false;
    data->restart_cursor = true;
    data->cursor = 0;

    return 0;
}

#define MM_INST(n)                                                                                 \
    static struct behavior_murtum_macro_data behavior_murtum_macro_data_##n = {};                      \
    static struct behavior_murtum_macro_config behavior_murtum_macro_config_##n = {                    \
        .index = n,                                                                                \
        .usage_pages = DT_INST_PROP(n, usage_pages),                                               \
        .usage_pages_count = DT_INST_PROP_LEN(n, usage_pages),                                     \
    };                                                                                             \
    BEHAVIOR_DT_INST_DEFINE(n, behavior_murtum_macro_init, NULL, &behavior_murtum_macro_data_##n,      \
                            &behavior_murtum_macro_config_##n, POST_KERNEL,                          \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &behavior_murtum_macro_driver_api);

DT_INST_FOREACH_STATUS_OKAY(MM_INST)

#endif
