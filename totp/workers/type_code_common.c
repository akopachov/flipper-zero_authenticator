#include "type_code_common.h"
#include <furi_hal_usb_hid.h>
#include <furi/core/kernel.h>
#include "../../services/convert/convert.h"
#include "../features_config.h"

#define KEYPRESS_DELAY (30)

static const uint8_t hid_number_keys[] = {
    HID_KEYBOARD_0, HID_KEYBOARD_1, HID_KEYBOARD_2, HID_KEYBOARD_3, HID_KEYBOARD_4,
    HID_KEYBOARD_5, HID_KEYBOARD_6, HID_KEYBOARD_7, HID_KEYBOARD_8, HID_KEYBOARD_9,
    HID_KEYBOARD_A, HID_KEYBOARD_B, HID_KEYBOARD_C, HID_KEYBOARD_D, HID_KEYBOARD_E,
    HID_KEYBOARD_F, HID_KEYBOARD_G, HID_KEYBOARD_H, HID_KEYBOARD_I, HID_KEYBOARD_J,
    HID_KEYBOARD_K, HID_KEYBOARD_L, HID_KEYBOARD_M, HID_KEYBOARD_N, HID_KEYBOARD_O,
    HID_KEYBOARD_P, HID_KEYBOARD_Q, HID_KEYBOARD_R, HID_KEYBOARD_S, HID_KEYBOARD_T,
    HID_KEYBOARD_U, HID_KEYBOARD_V, HID_KEYBOARD_W, HID_KEYBOARD_X, HID_KEYBOARD_Y,
    HID_KEYBOARD_Z};

static const uint8_t dictionary_alt_codes[] = { 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 
    65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90 };

static const uint8_t numpad_keys[] = {
    HID_KEYPAD_0,
    HID_KEYPAD_1,
    HID_KEYPAD_2,
    HID_KEYPAD_3,
    HID_KEYPAD_4,
    HID_KEYPAD_5,
    HID_KEYPAD_6,
    HID_KEYPAD_7,
    HID_KEYPAD_8,
    HID_KEYPAD_9,
};

static uint32_t get_keystroke_delay(TokenAutomationFeature features) {
    if(features & TokenAutomationFeatureTypeSlower) {
        return 100;
    }

    return 30;
}

static void totp_type_code_worker_press_key(
    uint16_t key,
    TOTP_AUTOMATION_KEY_HANDLER key_press_fn,
    TOTP_AUTOMATION_KEY_HANDLER key_release_fn) {
    (*key_press_fn)(key);
    furi_delay_ms(KEYPRESS_DELAY);
    (*key_release_fn)(key);
}

static void totp_type_code_worker_press_alt_code(
    uint8_t alt_code,
    TOTP_AUTOMATION_KEY_HANDLER key_press_fn,
    TOTP_AUTOMATION_KEY_HANDLER key_release_fn) {

    (*key_press_fn)(KEY_MOD_LEFT_ALT);

    if (alt_code >= 100) {
        totp_type_code_worker_press_key(numpad_keys[alt_code / 100], key_press_fn, key_release_fn);
    }

    if (alt_code >= 10) {
        totp_type_code_worker_press_key(numpad_keys[alt_code / 10 % 10], key_press_fn, key_release_fn);
    }

    if (alt_code > 0) {
        totp_type_code_worker_press_key(numpad_keys[alt_code % 10], key_press_fn, key_release_fn);
    }

    (*key_release_fn)(KEY_MOD_LEFT_ALT);
}

void totp_type_code_worker_execute_automation(
    TOTP_AUTOMATION_KEY_HANDLER key_press_fn,
    TOTP_AUTOMATION_KEY_HANDLER key_release_fn,
    TOTP_AUTOMATION_LED_STATE_HANDLER led_state_fn,
    const char* code_buffer,
    uint8_t code_buffer_size,
    TokenAutomationFeature features) {
    furi_delay_ms(500);

    bool numlock_state = led_state_fn == NULL || ((*led_state_fn)() & HID_KB_LED_NUM) == HID_KB_LED_NUM;

    if(!numlock_state) {
        totp_type_code_worker_press_key(HID_KEYBOARD_LOCK_NUM_LOCK, key_press_fn, key_release_fn);
    }

    uint8_t i = 0;
    char cb_char;
    while(i < code_buffer_size && (cb_char = code_buffer[i]) != 0) {
        uint8_t char_index = CONVERT_CHAR_TO_DIGIT(cb_char);
        if(char_index > 9) {
            char_index = cb_char - 'A' + 10;
        }

        if(char_index >= sizeof(dictionary_alt_codes)) break;

        if (led_state_fn != NULL) { // if we are sure we can control state of "NUM LOCK" key - use ALT-code approach
            uint8_t alt_code = dictionary_alt_codes[char_index];
            totp_type_code_worker_press_alt_code(alt_code, key_press_fn, key_release_fn);
        } else { // Otherwise use old approach
            uint16_t hid_kb_key = hid_number_keys[char_index];
            if(char_index > 9) {
                hid_kb_key |= KEY_MOD_LEFT_SHIFT;
            }

            totp_type_code_worker_press_key(hid_kb_key, key_press_fn, key_release_fn);
        }

        furi_delay_ms(get_keystroke_delay(features));
        i++;
    }

    if(!numlock_state) {
        totp_type_code_worker_press_key(HID_KEYBOARD_LOCK_NUM_LOCK, key_press_fn, key_release_fn);
    }

    if(features & TokenAutomationFeatureEnterAtTheEnd) {
        furi_delay_ms(get_keystroke_delay(features));
        totp_type_code_worker_press_key(HID_KEYBOARD_RETURN, key_press_fn, key_release_fn);
    }

    if(features & TokenAutomationFeatureTabAtTheEnd) {
        furi_delay_ms(get_keystroke_delay(features));
        totp_type_code_worker_press_key(HID_KEYBOARD_TAB, key_press_fn, key_release_fn);
    }
}