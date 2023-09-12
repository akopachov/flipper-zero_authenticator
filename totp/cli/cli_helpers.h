#pragma once

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TOTP_CLI_COMMAND_NAME "totp"

#define TOTP_CLI_COLOR_ERROR "91m"
#define TOTP_CLI_COLOR_WARNING "93m"
#define TOTP_CLI_COLOR_SUCCESS "92m"
#define TOTP_CLI_COLOR_INFO "96m"

#define TOTP_CLI_PRINTF(format, ...) printf(format, ##__VA_ARGS__)

#define TOTP_CLI_PRINTF_COLORFUL(color, format, ...) \
    TOTP_CLI_PRINTF("\e[%s" format "\e[0m", color, ##__VA_ARGS__)

#define TOTP_CLI_PRINTF_ERROR(format, ...) \
    TOTP_CLI_PRINTF_COLORFUL(TOTP_CLI_COLOR_ERROR, format, ##__VA_ARGS__)
#define TOTP_CLI_PRINTF_WARNING(format, ...) \
    TOTP_CLI_PRINTF_COLORFUL(TOTP_CLI_COLOR_WARNING, format, ##__VA_ARGS__)
#define TOTP_CLI_PRINTF_SUCCESS(format, ...) \
    TOTP_CLI_PRINTF_COLORFUL(TOTP_CLI_COLOR_SUCCESS, format, ##__VA_ARGS__)
#define TOTP_CLI_PRINTF_INFO(format, ...) \
    TOTP_CLI_PRINTF_COLORFUL(TOTP_CLI_COLOR_INFO, format, ##__VA_ARGS__)

#define TOTP_CLI_LOCK_UI(plugin_state)                                  \
    Scene __previous_scene = plugin_state->current_scene;               \
    totp_scene_director_activate_scene(plugin_state, TotpSceneStandby); \
    totp_scene_director_force_redraw(plugin_state)

#define TOTP_CLI_UNLOCK_UI(plugin_state)                                \
    totp_scene_director_activate_scene(plugin_state, __previous_scene); \
    totp_scene_director_force_redraw(plugin_state)

#define totp_cli_print_invalid_arguments() \
    TOTP_CLI_PRINTF_ERROR("Invalid command arguments. use \"help\" command to get list of available commands")

#define totp_cli_print_error_updating_config_file() \
    TOTP_CLI_PRINTF_ERROR("An error has occurred during updating config file\r\n")

#define totp_cli_print_error_loading_token_info() \
    TOTP_CLI_PRINTF_ERROR("An error has occurred during loading token information\r\n")

#define totp_cli_print_processing() \
    TOTP_CLI_PRINTF("Processing, please wait...\r\n")

#define totp_cli_delete_last_char() \
    TOTP_CLI_PRINTF("\b \b"); \
    fflush(stdout)

#define totp_cli_delete_current_line() \
    TOTP_CLI_PRINTF("\33[2K\r"); \
    fflush(stdout)

#define totp_cli_delete_last_line() \
    TOTP_CLI_PRINTF("\033[A\33[2K\r"); \
    fflush(stdout)

#ifdef __cplusplus
}
#endif