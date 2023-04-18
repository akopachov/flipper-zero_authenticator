#include "config.h"
#include <stdlib.h>
#include <string.h>
#include <flipper_format/flipper_format.h>
#include "../../types/common.h"
#include "../../types/token_info.h"
#include "../../features_config.h"
#include "migrations/common_migration.h"

#define CONFIG_FILE_DIRECTORY_PATH EXT_PATH("authenticator")
#define CONFIG_FILE_PATH CONFIG_FILE_DIRECTORY_PATH "/totp.conf"
#define CONFIG_FILE_BACKUP_BASE_PATH CONFIG_FILE_PATH ".backup"
#define CONFIG_FILE_TEMP_PATH CONFIG_FILE_PATH ".tmp"
#define CONFIG_FILE_ORIG_PATH CONFIG_FILE_PATH ".orig"
#define CONFIG_FILE_PATH_PREVIOUS EXT_PATH("apps/Misc") "/totp.conf"

/**
 * @brief Opens storage record
 * @return Storage record
 */
static Storage* totp_open_storage() {
    return furi_record_open(RECORD_STORAGE);
}

/**
 * @brief Closes storage record
 */
static void totp_close_storage() {
    furi_record_close(RECORD_STORAGE);
}

/**
 * @brief Closes config file
 * @param file config file reference
 */
static void totp_close_config_file(FlipperFormat* file) {
    if(file == NULL) return;
    flipper_format_file_close(file);
    flipper_format_free(file);
}

/**
 * @brief Tries to take a config file backup
 * @param storage storage record
 * @return backup path if backup successfully taken; \c NULL otherwise
 */
static char* totp_config_file_backup_i(Storage* storage) {
    uint8_t backup_path_size = sizeof(CONFIG_FILE_BACKUP_BASE_PATH) + 5;
    char* backup_path = malloc(backup_path_size);
    furi_check(backup_path != NULL);
    memcpy(backup_path, CONFIG_FILE_BACKUP_BASE_PATH, sizeof(CONFIG_FILE_BACKUP_BASE_PATH));
    uint16_t i = 1;
    bool backup_file_exists;
    while((backup_file_exists = storage_common_exists(storage, backup_path)) && i <= 9999) {
        snprintf(backup_path, backup_path_size, CONFIG_FILE_BACKUP_BASE_PATH ".%" PRIu16, i);
        i++;
    }

    if(backup_file_exists ||
       storage_common_copy(storage, CONFIG_FILE_PATH, backup_path) != FSE_OK) {
        FURI_LOG_E(LOGGING_TAG, "Unable to take a backup");
        free(backup_path);
        return NULL;
    }

    FURI_LOG_I(LOGGING_TAG, "Took config file backup to %s", backup_path);
    return backup_path;
}

/**
 * @brief Opens or creates TOTP application standard config file
 * @param storage storage record to use
 * @param[out] file opened config file
 * @return Config file open result
 */
static bool totp_open_config_file(Storage* storage, FlipperFormat** file) {
    FlipperFormat* fff_data_file = flipper_format_file_alloc(storage);

    if(storage_common_stat(storage, CONFIG_FILE_PATH, NULL) == FSE_OK) {
        FURI_LOG_D(LOGGING_TAG, "Config file %s found", CONFIG_FILE_PATH);
        if(!flipper_format_file_open_existing(fff_data_file, CONFIG_FILE_PATH)) {
            FURI_LOG_E(LOGGING_TAG, "Error opening existing file %s", CONFIG_FILE_PATH);
            totp_close_config_file(fff_data_file);
            return false;
        }
    } else if(storage_common_stat(storage, CONFIG_FILE_PATH_PREVIOUS, NULL) == FSE_OK) {
        FURI_LOG_D(LOGGING_TAG, "Old config file %s found", CONFIG_FILE_PATH_PREVIOUS);
        if(storage_common_stat(storage, CONFIG_FILE_DIRECTORY_PATH, NULL) == FSE_NOT_EXIST) {
            FURI_LOG_D(
                LOGGING_TAG,
                "Directory %s doesn't exist. Will create new.",
                CONFIG_FILE_DIRECTORY_PATH);
            if(!storage_simply_mkdir(storage, CONFIG_FILE_DIRECTORY_PATH)) {
                FURI_LOG_E(LOGGING_TAG, "Error creating directory %s", CONFIG_FILE_DIRECTORY_PATH);
                totp_close_config_file(fff_data_file);
                return false;
            }
        }
        if(storage_common_rename(storage, CONFIG_FILE_PATH_PREVIOUS, CONFIG_FILE_PATH) != FSE_OK) {
            FURI_LOG_E(LOGGING_TAG, "Error moving config to %s", CONFIG_FILE_PATH);
            totp_close_config_file(fff_data_file);
            return false;
        }
        FURI_LOG_I(LOGGING_TAG, "Applied config file path migration");
        return totp_open_config_file(storage, file);
    } else {
        FURI_LOG_D(LOGGING_TAG, "Config file %s is not found. Will create new.", CONFIG_FILE_PATH);
        if(storage_common_stat(storage, CONFIG_FILE_DIRECTORY_PATH, NULL) == FSE_NOT_EXIST) {
            FURI_LOG_D(
                LOGGING_TAG,
                "Directory %s doesn't exist. Will create new.",
                CONFIG_FILE_DIRECTORY_PATH);
            if(!storage_simply_mkdir(storage, CONFIG_FILE_DIRECTORY_PATH)) {
                FURI_LOG_E(LOGGING_TAG, "Error creating directory %s", CONFIG_FILE_DIRECTORY_PATH);
                return false;
            }
        }

        if(!flipper_format_file_open_new(fff_data_file, CONFIG_FILE_PATH)) {
            totp_close_config_file(fff_data_file);
            FURI_LOG_E(LOGGING_TAG, "Error creating new file %s", CONFIG_FILE_PATH);
            return false;
        }

        flipper_format_write_header_cstr(
            fff_data_file, CONFIG_FILE_HEADER, CONFIG_FILE_ACTUAL_VERSION);
        float tmp_tz = 0;
        flipper_format_write_comment_cstr(fff_data_file, " ");
        flipper_format_write_comment_cstr(
            fff_data_file,
            "Timezone offset in hours. Important note: do not put '+' sign for positive values");
        flipper_format_write_float(fff_data_file, TOTP_CONFIG_KEY_TIMEZONE, &tmp_tz, 1);

        uint32_t tmp_uint32 = NotificationMethodSound | NotificationMethodVibro;
        flipper_format_write_comment_cstr(fff_data_file, " ");
        flipper_format_write_comment_cstr(
            fff_data_file,
            "How to notify user when new token is generated or badusb mode is activated (possible values: 0 - do not notify, 1 - sound, 2 - vibro, 3 sound and vibro)");
        flipper_format_write_uint32(
            fff_data_file, TOTP_CONFIG_KEY_NOTIFICATION_METHOD, &tmp_uint32, 1);

        tmp_uint32 = AutomationMethodBadUsb;
        flipper_format_write_comment_cstr(fff_data_file, " ");
        flipper_format_write_comment_cstr(
            fff_data_file,
            "Automation method (0 - None, 1 - BadUSB, 2 - BadBT, 3 - BadUSB and BadBT)");
        flipper_format_write_uint32(
            fff_data_file, TOTP_CONFIG_KEY_AUTOMATION_METHOD, &tmp_uint32, 1);

        FuriString* temp_str = furi_string_alloc();

        flipper_format_write_comment_cstr(fff_data_file, " ");
        flipper_format_write_comment_cstr(fff_data_file, "=== TOKEN SAMPLE BEGIN ===");
        flipper_format_write_comment_cstr(fff_data_file, " ");
        flipper_format_write_comment_cstr(
            fff_data_file, "# Token name which will be visible in the UI.");
        furi_string_printf(temp_str, "%s: Sample token name", TOTP_CONFIG_KEY_TOKEN_NAME);
        flipper_format_write_comment(fff_data_file, temp_str);
        flipper_format_write_comment_cstr(fff_data_file, " ");

        flipper_format_write_comment_cstr(
            fff_data_file,
            "# Plain token secret without spaces, dashes and etc, just pure alpha-numeric characters. Important note: plain token will be encrypted and replaced by TOTP app");
        furi_string_printf(temp_str, "%s: plaintokensecret", TOTP_CONFIG_KEY_TOKEN_SECRET);
        flipper_format_write_comment(fff_data_file, temp_str);
        flipper_format_write_comment_cstr(fff_data_file, " ");

        furi_string_printf(
            temp_str,
            " # Token hashing algorithm to use during code generation. Supported options are %d (%s), %d (%s), %d (%s), and %d (%s). If you are not use which one to use - use %d (%s)",
            SHA1,
            TOTP_TOKEN_ALGO_SHA1_NAME,
            SHA256,
            TOTP_TOKEN_ALGO_SHA256_NAME,
            SHA512,
            TOTP_TOKEN_ALGO_SHA512_NAME,
            STEAM,
            TOTP_TOKEN_ALGO_STEAM_NAME,
            SHA1,
            TOTP_TOKEN_ALGO_SHA1_NAME);
        flipper_format_write_comment(fff_data_file, temp_str);
        furi_string_printf(
            temp_str, "%s: %d", TOTP_CONFIG_KEY_TOKEN_ALGO, SHA1);
        flipper_format_write_comment(fff_data_file, temp_str);
        flipper_format_write_comment_cstr(fff_data_file, " ");

        flipper_format_write_comment_cstr(
            fff_data_file,
            "# How many digits there should be in generated code. Available options are 5, 6 and 8. Majority websites requires 6 digits code, however some rare websites wants to get 8 digits code. If you are not sure which one to use - use 6");
        furi_string_printf(temp_str, "%s: 6", TOTP_CONFIG_KEY_TOKEN_DIGITS);
        flipper_format_write_comment(fff_data_file, temp_str);
        flipper_format_write_comment_cstr(fff_data_file, " ");

        flipper_format_write_comment_cstr(
            fff_data_file,
            "# Token lifetime duration in seconds. Should be between 15 and 255. Majority websites requires 30, however some rare websites may require custom lifetime. If you are not sure which one to use - use 30");
        furi_string_printf(temp_str, "%s: 30", TOTP_CONFIG_KEY_TOKEN_DURATION);
        flipper_format_write_comment(fff_data_file, temp_str);
        flipper_format_write_comment_cstr(fff_data_file, " ");

        flipper_format_write_comment_cstr(
            fff_data_file,
            "# Token input automation features (0 - None, 1 - press \"Enter\" key at the end of automation)");
        furi_string_printf(temp_str, "%s: 0", TOTP_CONFIG_KEY_TOKEN_AUTOMATION_FEATURES);
        flipper_format_write_comment(fff_data_file, temp_str);
        flipper_format_write_comment_cstr(fff_data_file, " ");

        flipper_format_write_comment_cstr(fff_data_file, "=== TOKEN SAMPLE END ===");
        flipper_format_write_comment_cstr(fff_data_file, " ");

        furi_string_free(temp_str);
        if(!flipper_format_rewind(fff_data_file)) {
            totp_close_config_file(fff_data_file);
            FURI_LOG_E(LOGGING_TAG, "Rewind error");
            return false;
        }
    }

    *file = fff_data_file;
    return true;
}

char* totp_config_file_backup() {
    Storage* storage = totp_open_storage();
    char* result = totp_config_file_backup_i(storage);
    totp_close_storage();
    return result;
}

bool totp_config_file_update_timezone_offset(const PluginState* plugin_state) {
    FlipperFormat* file = plugin_state->config_file_context->config_file;
    flipper_format_rewind(file);
    bool update_result = true;

    do {
        if(!flipper_format_insert_or_update_float(
                file, TOTP_CONFIG_KEY_TIMEZONE, &plugin_state->timezone_offset, 1)) {
            update_result = false;
            break;
        }

        update_result = true;
    } while(false);

    return update_result;
}

bool
    totp_config_file_update_notification_method(const PluginState* plugin_state) {
    FlipperFormat* file = plugin_state->config_file_context->config_file;
    flipper_format_rewind(file);
    bool update_result = true;

    do {
        uint32_t tmp_uint32 = plugin_state->notification_method;
        if(!flipper_format_insert_or_update_uint32(
                file, TOTP_CONFIG_KEY_NOTIFICATION_METHOD, &tmp_uint32, 1)) {
            update_result = false;
            break;
        }

        update_result = true;
    } while(false);

    return update_result;
}

bool
    totp_config_file_update_automation_method(const PluginState* plugin_state) {
    FlipperFormat* file = plugin_state->config_file_context->config_file;
    flipper_format_rewind(file);
    bool update_result = true;

    do {
        uint32_t tmp_uint32 = plugin_state->automation_method;
        if(!flipper_format_insert_or_update_uint32(
                file, TOTP_CONFIG_KEY_AUTOMATION_METHOD, &tmp_uint32, 1)) {
            update_result = false;
            break;
        }

        update_result = true;
    } while(false);

    return update_result;
}

bool totp_config_file_update_user_settings(const PluginState* plugin_state) {
    FlipperFormat* file = plugin_state->config_file_context->config_file;
    flipper_format_rewind(file);
    bool update_result = true;
    do {
        if(!flipper_format_insert_or_update_float(
                file, TOTP_CONFIG_KEY_TIMEZONE, &plugin_state->timezone_offset, 1)) {
            update_result = false;
            break;
        }
        uint32_t tmp_uint32 = plugin_state->notification_method;
        if(!flipper_format_insert_or_update_uint32(
                file, TOTP_CONFIG_KEY_NOTIFICATION_METHOD, &tmp_uint32, 1)) {
            update_result = false;
            break;
        }

        tmp_uint32 = plugin_state->automation_method;
        if(!flipper_format_insert_or_update_uint32(
                file, TOTP_CONFIG_KEY_AUTOMATION_METHOD, &tmp_uint32, 1)) {
            update_result = false;
            break;
        }

        update_result = true;
    } while(false);

    return update_result;
}

bool totp_config_file_load(PluginState* const plugin_state) {
    Storage* storage = totp_open_storage();
    FlipperFormat* fff_data_file;
    if (!totp_open_config_file(storage, &fff_data_file)) {
        totp_close_storage();
        return false;
    }

    flipper_format_rewind(fff_data_file);

    bool result = true;

    plugin_state->timezone_offset = 0;

    FuriString* temp_str = furi_string_alloc();

    do {
        uint32_t file_version;
        if(!flipper_format_read_header(fff_data_file, temp_str, &file_version)) {
            FURI_LOG_E(LOGGING_TAG, "Missing or incorrect header");
            FURI_LOG_D(LOGGING_TAG, "Line 339");
            result = false;
            break;
        }

        if(file_version < CONFIG_FILE_ACTUAL_VERSION) {
            FURI_LOG_I(
                LOGGING_TAG,
                "Obsolete config file version detected. Current version: %" PRIu32
                "; Actual version: %" PRId16,
                file_version,
                CONFIG_FILE_ACTUAL_VERSION);
            totp_close_config_file(fff_data_file);

            char* backup_path = totp_config_file_backup_i(storage);

            if(backup_path != NULL) {
                if(totp_open_config_file(storage, &fff_data_file) != true) {
                    result = false;
                    break;
                }

                FlipperFormat* fff_backup_data_file = flipper_format_file_alloc(storage);
                if(!flipper_format_file_open_existing(fff_backup_data_file, backup_path)) {
                    flipper_format_file_close(fff_backup_data_file);
                    flipper_format_free(fff_backup_data_file);
                    result = false;
                    break;
                }

                if(totp_config_migrate_to_latest(fff_data_file, fff_backup_data_file)) {
                    FURI_LOG_I(
                        LOGGING_TAG,
                        "Applied migration to version %" PRId16,
                        CONFIG_FILE_ACTUAL_VERSION);
                    file_version = CONFIG_FILE_ACTUAL_VERSION;
                } else {
                    FURI_LOG_W(
                        LOGGING_TAG,
                        "An error occurred during migration to version %" PRId16,
                        CONFIG_FILE_ACTUAL_VERSION);
                    result = false;
                    break;
                }

                flipper_format_file_close(fff_backup_data_file);
                flipper_format_free(fff_backup_data_file);
                flipper_format_rewind(fff_data_file);
                free(backup_path);
            } else {
                FURI_LOG_E(
                    LOGGING_TAG,
                    "An error occurred during taking backup of %s before migration",
                    CONFIG_FILE_PATH);
                result = false;
                break;
            }
        }

        if(!flipper_format_read_hex(
               fff_data_file, TOTP_CONFIG_KEY_BASE_IV, &plugin_state->base_iv[0], TOTP_IV_SIZE)) {
            FURI_LOG_D(LOGGING_TAG, "Missing base IV");
        }

        if(!flipper_format_rewind(fff_data_file)) {
            result = false;
            break;
        }

        uint32_t crypto_size;
        if(flipper_format_get_value_count(
               fff_data_file, TOTP_CONFIG_KEY_CRYPTO_VERIFY, &crypto_size) &&
           crypto_size > 0) {
            plugin_state->crypto_verify_data = malloc(sizeof(uint8_t) * crypto_size);
            furi_check(plugin_state->crypto_verify_data != NULL);
            plugin_state->crypto_verify_data_length = crypto_size;
            if(!flipper_format_read_hex(
                   fff_data_file,
                   TOTP_CONFIG_KEY_CRYPTO_VERIFY,
                   plugin_state->crypto_verify_data,
                   crypto_size)) {
                FURI_LOG_D(LOGGING_TAG, "Missing crypto verify token");
                free(plugin_state->crypto_verify_data);
                plugin_state->crypto_verify_data = NULL;
                plugin_state->crypto_verify_data_length = 0;
            }
        } else {
            plugin_state->crypto_verify_data = NULL;
            plugin_state->crypto_verify_data_length = 0;
        }

        if(!flipper_format_rewind(fff_data_file)) {
            result = false;
            break;
        }

        if(!flipper_format_read_float(
               fff_data_file, TOTP_CONFIG_KEY_TIMEZONE, &plugin_state->timezone_offset, 1)) {
            plugin_state->timezone_offset = 0;
            FURI_LOG_D(LOGGING_TAG, "Missing timezone offset information, defaulting to 0");
        }

        if(!flipper_format_rewind(fff_data_file)) {
            result = false;
            break;
        }

        if(!flipper_format_read_bool(
               fff_data_file, TOTP_CONFIG_KEY_PINSET, &plugin_state->pin_set, 1)) {
            plugin_state->pin_set = true;
        }

        flipper_format_rewind(fff_data_file);

        uint32_t tmp_uint32;
        if(!flipper_format_read_uint32(
               fff_data_file, TOTP_CONFIG_KEY_NOTIFICATION_METHOD, &tmp_uint32, 1)) {
            tmp_uint32 = NotificationMethodSound | NotificationMethodVibro;
        }

        plugin_state->notification_method = tmp_uint32;

        flipper_format_rewind(fff_data_file);

        if(!flipper_format_read_uint32(
               fff_data_file, TOTP_CONFIG_KEY_AUTOMATION_METHOD, &tmp_uint32, 1)) {
            tmp_uint32 = AutomationMethodBadUsb;
        }

        plugin_state->automation_method = tmp_uint32;

        plugin_state->config_file_context = malloc(sizeof(ConfigFileContext));
        furi_check(plugin_state->config_file_context != NULL);
        plugin_state->config_file_context->config_file = fff_data_file;
        plugin_state->config_file_context->token_info_iterator_context = totp_token_info_iterator_alloc(plugin_state->config_file_context->config_file, plugin_state->iv);
    } while(false);

    furi_string_free(temp_str);
    return result;
}

bool
    totp_config_file_update_crypto_signatures(const PluginState* plugin_state) {
    FlipperFormat* config_file = plugin_state->config_file_context->config_file;
    flipper_format_rewind(config_file);
    bool update_result = true;
    do {
        if(!flipper_format_insert_or_update_hex(
                config_file, TOTP_CONFIG_KEY_BASE_IV, plugin_state->base_iv, TOTP_IV_SIZE)) {
            update_result = false;
            break;
        }

        if(!flipper_format_insert_or_update_hex(
                config_file,
                TOTP_CONFIG_KEY_CRYPTO_VERIFY,
                plugin_state->crypto_verify_data,
                plugin_state->crypto_verify_data_length)) {
            update_result = false;
            break;
        }

        if(!flipper_format_insert_or_update_bool(
                config_file, TOTP_CONFIG_KEY_PINSET, &plugin_state->pin_set, 1)) {
            update_result = false;
            break;
        }

        update_result = true;
    } while(false);

    return update_result;
}

void totp_config_file_close(const PluginState* plugin_state) {
    if (plugin_state->config_file_context == NULL) return;
    totp_token_info_iterator_free(plugin_state->config_file_context->token_info_iterator_context);
    totp_close_config_file(plugin_state->config_file_context->config_file);
    free(plugin_state->config_file_context);
}

void totp_config_file_reset(const PluginState* plugin_state) {
    totp_config_file_close(plugin_state);
    Storage* storage = totp_open_storage();
    storage_simply_remove(storage, CONFIG_FILE_PATH);
    totp_close_storage();
}