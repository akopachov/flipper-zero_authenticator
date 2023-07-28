#pragma once

#include "../features_config.h"

typedef uint8_t AutomationMethod;
typedef uint8_t AutomationKeyboardLayout;

enum AutomationMethods {
    AutomationMethodNone = 0b00,
    AutomationMethodBadUsb = 0b01,
#ifdef TOTP_BADBT_TYPE_ENABLED
    AutomationMethodBadBt = 0b10,
#endif
};

enum AutomationKeyboardLayouts {
    AutomationKeyboardLayoutQWERTY = 0,
    AutomationKeyboardLayoutAZERTY = 1
};