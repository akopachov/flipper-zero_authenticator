#pragma once

#include "../../types/plugin_state.h"
#include "../../types/token_info.h"
#include "config_file_context.h"
#include "constants.h"

typedef uint8_t TotpConfigFileOpenResult;
typedef uint8_t TotpConfigFileUpdateResult;

/**
 * @brief Tries to take a config file backup
 * @return backup path if backup successfully taken; \c NULL otherwise
 */
char* totp_config_file_backup();

/**
 * @brief Loads basic information from an application config file into application state without loading all the tokens
 * @param plugin_state application state
 * @return Config file open result
 */
bool totp_config_file_load(PluginState* const plugin_state);

/**
 * @brief Updates timezone offset in an application config file
 * @param new_timezone_offset new timezone offset to be set
 * @return Config file update result
 */
bool totp_config_file_update_timezone_offset(const PluginState* plugin_state);

/**
 * @brief Updates notification method in an application config file
 * @param new_notification_method new notification method to be set
 * @return Config file update result
 */
bool
    totp_config_file_update_notification_method(const PluginState* plugin_state);

/**
 * @brief Updates automation method in an application config file
 * @param new_automation_method new automation method to be set
 * @return Config file update result
 */
bool
    totp_config_file_update_automation_method(const PluginState* plugin_state);

/**
 * @brief Updates application user settings
 * @param plugin_state application state
 * @return Config file update result
 */
bool totp_config_file_update_user_settings(const PluginState* plugin_state);

/**
 * @brief Updates crypto signatures information
 * @param plugin_state application state
 * @return Config file update result
 */
bool
    totp_config_file_update_crypto_signatures(const PluginState* plugin_state);

/**
 * @brief Reset all the settings to default
 */
void totp_config_file_reset(PluginState* const plugin_state);

void totp_config_file_close(PluginState* const plugin_state);