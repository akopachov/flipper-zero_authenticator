// Include Bluetooth token input automation
#ifndef TOTP_NO_BADBT_TYPE
#define TOTP_BADBT_TYPE_ENABLED
#endif

// Include token input automation icons on the main screen
#ifndef TOTP_NO_AUTOMATION_ICONS
#define TOTP_AUTOMATION_ICONS_ENABLED
#endif

// List of compatible firmwares
#define TOTP_FIRMWARE_OFFICIAL_STABLE (1)
#define TOTP_FIRMWARE_OFFICIAL_DEV (2)
#define TOTP_FIRMWARE_XTREME (3)
// End of list

// Target firmware to build for.
#ifndef TOTP_TARGET_FIRMWARE
// Defaulting to Xtreme if not previously defined
#define TOTP_TARGET_FIRMWARE TOTP_FIRMWARE_XTREME
#endif