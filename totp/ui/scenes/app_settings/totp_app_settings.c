#include "totp_app_settings.h"
#include <math.h>
#include <totp_icons.h>
#include <available_fonts.h>
#include "../../canvas_extensions.h"
#include "../../ui_controls.h"
#include "../../common_dialogs.h"
#include "../../scene_director.h"
#include "../../constants.h"
#include "../../../services/config/config.h"
#include "../../../services/convert/convert.h"
#include <roll_value.h>
#include "../../../config/app/config.h"
#ifdef TOTP_BADBT_AUTOMATION_ENABLED
#include "../../../workers/bt_type_code/bt_type_code.h"
#endif

#ifdef TOTP_BADBT_AUTOMATION_ENABLED
#define AUTOMATION_LIST_MAX_INDEX (3)
#else
#define AUTOMATION_LIST_MAX_INDEX (1)
#endif
#define BAD_KB_LAYOUT_LIST_MAX_INDEX (1)
#define FONT_TEST_STR_LENGTH (7)

static const char* YES_NO_LIST[] = {"NO", "YES"};
static const char* AUTOMATION_LIST[] = {
    "None",
    "USB"
#ifdef TOTP_BADBT_AUTOMATION_ENABLED
    ,
    "Bluetooth",
    "BT and USB"
#endif
};
static const char* BAD_KB_LAYOUT_LIST[] = {"QWERTY", "AZERTY"};
static const char* FONT_TEST_STR = "0123BCD";

typedef enum {
    HoursInput,
    MinutesInput,
    FontSelect,
    SoundSwitch,
    VibroSwitch,
    AutomationSwitch,
    BadKeyboardLayoutSelect,
    ConfirmButton
} Control;

typedef struct {
    int8_t tz_offset_hours;
    uint8_t tz_offset_minutes;
    bool notification_sound;
    bool notification_vibro;
    AutomationMethod automation_method;
    uint16_t y_offset;
    AutomationKeyboardLayout automation_kb_layout;
    Control selected_control;
    uint8_t active_font;
} SceneState;

void totp_scene_app_settings_activate(PluginState* plugin_state) {
    SceneState* scene_state = malloc(sizeof(SceneState));
    furi_check(scene_state != NULL);
    plugin_state->current_scene_state = scene_state;

    float off_int;
    float off_dec = modff(plugin_state->timezone_offset, &off_int);
    scene_state->tz_offset_hours = off_int;
    scene_state->tz_offset_minutes = 60.0f * off_dec;
    scene_state->notification_sound = plugin_state->notification_method & NotificationMethodSound;
    scene_state->notification_vibro = plugin_state->notification_method & NotificationMethodVibro;
    scene_state->automation_method =
        MIN(plugin_state->automation_method, AUTOMATION_LIST_MAX_INDEX);
    scene_state->automation_kb_layout =
        MIN(plugin_state->automation_kb_layout, BAD_KB_LAYOUT_LIST_MAX_INDEX);

    scene_state->active_font = plugin_state->active_font_index;
}

static void two_digit_to_str(int8_t num, char* str) {
    char* s = str;
    if(num < 0) {
        *(s++) = '-';
        num = -num;
    }

    uint8_t d1 = (num / 10) % 10;
    uint8_t d2 = num % 10;
    *(s++) = CONVERT_DIGIT_TO_CHAR(d1);
    *(s++) = CONVERT_DIGIT_TO_CHAR(d2);
    *(s++) = '\0';
}

void totp_scene_app_settings_render(Canvas* const canvas, const PluginState* plugin_state) {
    const SceneState* scene_state = plugin_state->current_scene_state;
    if(scene_state->selected_control < FontSelect) {
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(
            canvas, 0, 0 - scene_state->y_offset, AlignLeft, AlignTop, "Timezone offset");
        canvas_set_font(canvas, FontSecondary);

        char tmp_str[4];
        two_digit_to_str(scene_state->tz_offset_hours, &tmp_str[0]);
        canvas_draw_str_aligned(
            canvas, 0, 17 - scene_state->y_offset, AlignLeft, AlignTop, "Hours:");
        ui_control_select_render(
            canvas,
            36,
            10 - scene_state->y_offset,
            SCREEN_WIDTH - 36 - UI_CONTROL_VSCROLL_WIDTH,
            &tmp_str[0],
            scene_state->selected_control == HoursInput);

        two_digit_to_str(scene_state->tz_offset_minutes, &tmp_str[0]);
        canvas_draw_str_aligned(
            canvas, 0, 35 - scene_state->y_offset, AlignLeft, AlignTop, "Minutes:");
        ui_control_select_render(
            canvas,
            36,
            28 - scene_state->y_offset,
            SCREEN_WIDTH - 36 - UI_CONTROL_VSCROLL_WIDTH,
            &tmp_str[0],
            scene_state->selected_control == MinutesInput);

    } else if(scene_state->selected_control < SoundSwitch) {
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(
            canvas, 0, 64 - scene_state->y_offset, AlignLeft, AlignTop, "Font");
        canvas_set_font(canvas, FontSecondary);

        const FONT_INFO* const font = available_fonts[scene_state->active_font];
        ui_control_select_render(
            canvas,
            0,
            74 - scene_state->y_offset,
            SCREEN_WIDTH - UI_CONTROL_VSCROLL_WIDTH,
            font->name,
            scene_state->selected_control == FontSelect);

        uint8_t font_x_offset =
            SCREEN_WIDTH_CENTER -
            (((font->charInfo[0].width + font->spacePixels) * FONT_TEST_STR_LENGTH) >> 1);
        uint8_t font_y_offset = 108 - scene_state->y_offset - (font->height >> 1);
        canvas_draw_str_ex(
            canvas, font_x_offset, font_y_offset, FONT_TEST_STR, FONT_TEST_STR_LENGTH, font);

    } else if(scene_state->selected_control < AutomationSwitch) {
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(
            canvas, 0, 128 - scene_state->y_offset, AlignLeft, AlignTop, "Notifications");
        canvas_set_font(canvas, FontSecondary);

        canvas_draw_str_aligned(
            canvas, 0, 145 - scene_state->y_offset, AlignLeft, AlignTop, "Sound:");
        ui_control_select_render(
            canvas,
            36,
            138 - scene_state->y_offset,
            SCREEN_WIDTH - 36 - UI_CONTROL_VSCROLL_WIDTH,
            YES_NO_LIST[scene_state->notification_sound],
            scene_state->selected_control == SoundSwitch);

        canvas_draw_str_aligned(
            canvas, 0, 163 - scene_state->y_offset, AlignLeft, AlignTop, "Vibro:");
        ui_control_select_render(
            canvas,
            36,
            156 - scene_state->y_offset,
            SCREEN_WIDTH - 36 - UI_CONTROL_VSCROLL_WIDTH,
            YES_NO_LIST[scene_state->notification_vibro],
            scene_state->selected_control == VibroSwitch);
    } else {
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(
            canvas, 0, 192 - scene_state->y_offset, AlignLeft, AlignTop, "Automation");
        canvas_set_font(canvas, FontSecondary);

        canvas_draw_str_aligned(
            canvas, 0, 209 - scene_state->y_offset, AlignLeft, AlignTop, "Method:");
        ui_control_select_render(
            canvas,
            36,
            202 - scene_state->y_offset,
            SCREEN_WIDTH - 36 - UI_CONTROL_VSCROLL_WIDTH,
            AUTOMATION_LIST[scene_state->automation_method],
            scene_state->selected_control == AutomationSwitch);

        canvas_draw_str_aligned(
            canvas, 0, 227 - scene_state->y_offset, AlignLeft, AlignTop, "Layout:");

        ui_control_select_render(
            canvas,
            36,
            220 - scene_state->y_offset,
            SCREEN_WIDTH - 36 - UI_CONTROL_VSCROLL_WIDTH,
            BAD_KB_LAYOUT_LIST[scene_state->automation_kb_layout],
            scene_state->selected_control == BadKeyboardLayoutSelect);

        ui_control_button_render(
            canvas,
            SCREEN_WIDTH_CENTER - 24,
            242 - scene_state->y_offset,
            48,
            13,
            "Confirm",
            scene_state->selected_control == ConfirmButton);
    }

    ui_control_vscroll_render(
        canvas, SCREEN_WIDTH - 3, 0, SCREEN_HEIGHT, scene_state->selected_control, ConfirmButton);
}

bool totp_scene_app_settings_handle_event(
    const PluginEvent* const event,
    PluginState* plugin_state) {
    if(event->type != EventTypeKey) {
        return true;
    }

    SceneState* scene_state = (SceneState*)plugin_state->current_scene_state;
    if(event->input.type == InputTypePress || event->input.type == InputTypeRepeat) {
        switch(event->input.key) {
        case InputKeyUp:
            totp_roll_value_uint8_t(
                &scene_state->selected_control,
                -1,
                HoursInput,
                ConfirmButton,
                RollOverflowBehaviorStop);
            if(scene_state->selected_control > VibroSwitch) {
                scene_state->y_offset = SCREEN_HEIGHT * 3;
            } else if(scene_state->selected_control > FontSelect) {
                scene_state->y_offset = SCREEN_HEIGHT * 2;
            } else if(scene_state->selected_control > MinutesInput) {
                scene_state->y_offset = SCREEN_HEIGHT;
            } else {
                scene_state->y_offset = 0;
            }
            break;
        case InputKeyDown:
            totp_roll_value_uint8_t(
                &scene_state->selected_control,
                1,
                HoursInput,
                ConfirmButton,
                RollOverflowBehaviorStop);
            if(scene_state->selected_control > VibroSwitch) {
                scene_state->y_offset = SCREEN_HEIGHT * 3;
            } else if(scene_state->selected_control > FontSelect) {
                scene_state->y_offset = SCREEN_HEIGHT * 2;
            } else if(scene_state->selected_control > MinutesInput) {
                scene_state->y_offset = SCREEN_HEIGHT;
            } else {
                scene_state->y_offset = 0;
            }
            break;
        case InputKeyRight:
            if(scene_state->selected_control == HoursInput) {
                totp_roll_value_int8_t(
                    &scene_state->tz_offset_hours, 1, -12, 12, RollOverflowBehaviorStop);
            } else if(scene_state->selected_control == MinutesInput) {
                totp_roll_value_uint8_t(
                    &scene_state->tz_offset_minutes, 15, 0, 45, RollOverflowBehaviorRoll);
            } else if(scene_state->selected_control == FontSelect) {
                totp_roll_value_uint8_t(
                    &scene_state->active_font,
                    1,
                    0,
                    AVAILABLE_FONTS_COUNT - 1,
                    RollOverflowBehaviorRoll);
            } else if(scene_state->selected_control == SoundSwitch) {
                scene_state->notification_sound = !scene_state->notification_sound;
            } else if(scene_state->selected_control == VibroSwitch) {
                scene_state->notification_vibro = !scene_state->notification_vibro;
            } else if(scene_state->selected_control == AutomationSwitch) {
                totp_roll_value_uint8_t(
                    &scene_state->automation_method,
                    1,
                    0,
                    AUTOMATION_LIST_MAX_INDEX,
                    RollOverflowBehaviorRoll);
            } else if(scene_state->selected_control == BadKeyboardLayoutSelect) {
                totp_roll_value_uint8_t(
                    &scene_state->automation_kb_layout,
                    1,
                    0,
                    BAD_KB_LAYOUT_LIST_MAX_INDEX,
                    RollOverflowBehaviorRoll);
            }
            break;
        case InputKeyLeft:
            if(scene_state->selected_control == HoursInput) {
                totp_roll_value_int8_t(
                    &scene_state->tz_offset_hours, -1, -12, 12, RollOverflowBehaviorStop);
            } else if(scene_state->selected_control == MinutesInput) {
                totp_roll_value_uint8_t(
                    &scene_state->tz_offset_minutes, -15, 0, 45, RollOverflowBehaviorRoll);
            } else if(scene_state->selected_control == FontSelect) {
                totp_roll_value_uint8_t(
                    &scene_state->active_font,
                    -1,
                    0,
                    AVAILABLE_FONTS_COUNT - 1,
                    RollOverflowBehaviorRoll);
            } else if(scene_state->selected_control == SoundSwitch) {
                scene_state->notification_sound = !scene_state->notification_sound;
            } else if(scene_state->selected_control == VibroSwitch) {
                scene_state->notification_vibro = !scene_state->notification_vibro;
            } else if(scene_state->selected_control == AutomationSwitch) {
                totp_roll_value_uint8_t(
                    &scene_state->automation_method,
                    -1,
                    0,
                    AUTOMATION_LIST_MAX_INDEX,
                    RollOverflowBehaviorRoll);
            } else if(scene_state->selected_control == BadKeyboardLayoutSelect) {
                totp_roll_value_uint8_t(
                    &scene_state->automation_kb_layout,
                    -1,
                    0,
                    BAD_KB_LAYOUT_LIST_MAX_INDEX,
                    RollOverflowBehaviorRoll);
            }
            break;
        case InputKeyOk:
            break;
        case InputKeyBack: {
            totp_scene_director_activate_scene(plugin_state, TotpSceneTokenMenu);
            break;
        }
        default:
            break;
        }
    } else if(
        event->input.type == InputTypeRelease && event->input.key == InputKeyOk &&
        scene_state->selected_control == ConfirmButton) {
        plugin_state->timezone_offset =
            (float)scene_state->tz_offset_hours + (float)scene_state->tz_offset_minutes / 60.0f;

        plugin_state->notification_method =
            (scene_state->notification_sound ? NotificationMethodSound : NotificationMethodNone) |
            (scene_state->notification_vibro ? NotificationMethodVibro : NotificationMethodNone);

        plugin_state->automation_method = scene_state->automation_method;
        plugin_state->active_font_index = scene_state->active_font;
        plugin_state->automation_kb_layout = scene_state->automation_kb_layout;

        if(!totp_config_file_update_user_settings(plugin_state)) {
            totp_dialogs_config_updating_error(plugin_state);
            return false;
        }

#ifdef TOTP_BADBT_AUTOMATION_ENABLED
        if((scene_state->automation_method & AutomationMethodBadBt) == 0 &&
           plugin_state->bt_type_code_worker_context != NULL) {
            totp_bt_type_code_worker_free(plugin_state->bt_type_code_worker_context);
            plugin_state->bt_type_code_worker_context = NULL;
        }
#endif

        totp_scene_director_activate_scene(plugin_state, TotpSceneTokenMenu);
    }

    return true;
}

void totp_scene_app_settings_deactivate(PluginState* plugin_state) {
    if(plugin_state->current_scene_state == NULL) return;

    free(plugin_state->current_scene_state);
    plugin_state->current_scene_state = NULL;
}
