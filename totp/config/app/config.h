// Application automatic lock timeout if user IDLE. (ticks)
#ifndef TOTP_AUTO_LOCK_IDLE_TIMEOUT_SEC
#define TOTP_AUTO_LOCK_IDLE_TIMEOUT_SEC (60)
#endif

// Enables\disables Bluetooth token input automation
#ifndef TOTP_NO_BADBT_AUTOMATION
#define TOTP_BADBT_AUTOMATION_ENABLED
#endif

// Enables\disables backward compatibility with crypto algorithms v1
#ifndef TOTP_NO_OBSOLETE_CRYPTO_V1_COMPATIBILITY
#define TOTP_OBSOLETE_CRYPTO_V1_COMPATIBILITY_ENABLED
#endif

// Enables\disables backward compatibility with crypto algorithms v2
#ifndef TOTP_NO_OBSOLETE_CRYPTO_V2_COMPATIBILITY
#define TOTP_OBSOLETE_CRYPTO_V2_COMPATIBILITY_ENABLED
#endif

// Enables\disables userfriendly TOTP CLI help text
// If disabled, it will print a link to a wiki page
#ifndef TOTP_CLI_NO_RICH_HELP
#define TOTP_CLI_RICH_HELP_ENABLED
#endif

// Enables\disables "Add new token" UI
// If disabled it will print a link to wiki page
#ifndef TOTP_UI_NO_ADD_NEW_TOKEN
#define TOTP_UI_ADD_NEW_TOKEN_ENABLED
#endif

// List of compatible firmwares
#define TOTP_FIRMWARE_OFFICIAL_STABLE (1)
#define TOTP_FIRMWARE_OFFICIAL_DEV (2)
#define TOTP_FIRMWARE_XTREME_UL (3)
// End of list

// Checking FlipC.org definitions (https://github.com/playmean/fap-list/issues/9)
#if defined(TARGET_FIRMWARE_OFFICIAL)
#define TOTP_TARGET_FIRMWARE TOTP_FIRMWARE_OFFICIAL_STABLE
#elif defined(TARGET_FIRMWARE_UNLEASHED)
#define TOTP_TARGET_FIRMWARE TOTP_FIRMWARE_XTREME_UL
#elif defined(TARGET_FIRMWARE_XTREME)
#define TOTP_TARGET_FIRMWARE TOTP_FIRMWARE_XTREME_UL
#endif
// End of FlipC.org definition checks

// If target firmware is not yet set, default it to OFW because there is no chance to force Flipper Devices to update their build pipelines :angry:.
// I'm still using Xtreme firmware, it is still the best one and I highly recommend it to everybody.
#ifndef TOTP_TARGET_FIRMWARE
#define TOTP_TARGET_FIRMWARE TOTP_FIRMWARE_OFFICIAL_STABLE
#endif
