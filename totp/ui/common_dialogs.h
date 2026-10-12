#pragma once

#include <dialogs/dialogs.h>
#include "../types/plugin_state.h"

/**
 * @brief Shows standard dialog about the fact that error occurred when loading config file
 * @param plugin_state application state
 * @return dialog button which user pressed to close the dialog
 */
DialogMessageButton totp_dialogs_config_loading_error(PluginState* plugin_state);

/**
 * @brief Shows standard dialog about the fact that error occurred when updating config file
 * @param plugin_state application state
 * @return dialog button which user pressed to close the dialog
 */
DialogMessageButton totp_dialogs_config_updating_error(PluginState* plugin_state);

/**
 * @brief Shows dialog about the fact that config file uses unsupported crypto version and offers to reset it
 * @param plugin_state application state
 * @return \c DialogMessageButtonRight if user agreed to reset config file; other dialog button otherwise
 */
DialogMessageButton totp_dialogs_config_unsupported_crypto_version(PluginState* plugin_state);