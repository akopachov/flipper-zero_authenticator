#include "update.h"
#include <stdlib.h>
#include <lib/toolbox/args.h>
#include "../../../lib/list/list.h"
#include "../../../types/token_info.h"
#include "../../../services/config/config.h"
#include "../../../services/convert/convert.h"
#include "../../cli_helpers.h"
#include "../../../ui/scene_director.h"
#include "../../common_command_arguments.h"

#define TOTP_CLI_COMMAND_UPDATE_ARG_SECRET_PREFIX "-s"

void totp_cli_command_update_docopt_commands() {
    TOTP_CLI_PRINTF("  " TOTP_CLI_COMMAND_UPDATE "           Update existing token\r\n");
}

void totp_cli_command_update_docopt_usage() {
    TOTP_CLI_PRINTF(
        "  " TOTP_CLI_COMMAND_NAME
        " " DOCOPT_REQUIRED(TOTP_CLI_COMMAND_UPDATE) " " DOCOPT_ARGUMENT(TOTP_CLI_COMMAND_ARG_INDEX) " " DOCOPT_OPTIONAL(DOCOPT_OPTION(TOTP_CLI_COMMAND_ARG_ALGO_PREFIX, DOCOPT_ARGUMENT(TOTP_CLI_COMMAND_ARG_ALGO))) " " DOCOPT_OPTIONAL(DOCOPT_OPTION(TOTP_CLI_COMMAND_ARG_NAME_PREFIX, DOCOPT_ARGUMENT(TOTP_CLI_COMMAND_ARG_NAME))) " " DOCOPT_OPTIONAL(
            DOCOPT_OPTION(
                TOTP_CLI_COMMAND_ARG_DIGITS_PREFIX,
                DOCOPT_ARGUMENT(
                    TOTP_CLI_COMMAND_ARG_DIGITS))) " " DOCOPT_OPTIONAL(DOCOPT_OPTION(TOTP_CLI_COMMAND_ARG_DURATION_PREFIX, DOCOPT_ARGUMENT(TOTP_CLI_COMMAND_ARG_DURATION))) " " DOCOPT_OPTIONAL(DOCOPT_SWITCH(TOTP_CLI_COMMAND_ARG_UNSECURE_PREFIX)) " " DOCOPT_MULTIPLE(DOCOPT_OPTIONAL(DOCOPT_OPTION(TOTP_CLI_COMMAND_ARG_AUTOMATION_FEATURE_PREFIX, DOCOPT_ARGUMENT(TOTP_CLI_COMMAND_ARG_AUTOMATION_FEATURE)))) "\r\n");
}

void totp_cli_command_update_docopt_options() {
    TOTP_CLI_PRINTF("  " DOCOPT_OPTION(
        TOTP_CLI_COMMAND_ARG_NAME_PREFIX,
        DOCOPT_ARGUMENT(TOTP_CLI_COMMAND_ARG_NAME)) "      Token name\r\n");
}

void totp_cli_command_update_handle(PluginState* plugin_state, FuriString* args, Cli* cli) {
    FuriString* temp_str = furi_string_alloc();

    if(!totp_cli_ensure_authenticated(plugin_state, cli)) {
        return;
    }

    int token_number;
    if(!args_read_int_and_trim(args, &token_number) || token_number <= 0 ||
       token_number > plugin_state->tokens_count) {
        TOTP_CLI_PRINT_INVALID_ARGUMENTS();
        return;
    }

    ListNode* list_item = list_element_at(plugin_state->tokens_list, token_number - 1);
    TokenInfo* existing_token_info = list_item->data;
    TokenInfo* token_info = token_info_clone(existing_token_info);

    // Read optional arguments
    bool mask_user_input = true;
    bool update_token_secret = false;
    while(args_read_string_and_trim(args, temp_str)) {
        bool parsed = false;
        if(furi_string_cmpi_str(temp_str, TOTP_CLI_COMMAND_ARG_NAME_PREFIX) == 0) {
            if(!args_read_probably_quoted_string_and_trim(args, temp_str) ||
               furi_string_empty(temp_str)) {
                TOTP_CLI_PRINTF_ERROR(
                    "Missed value for argument \"" TOTP_CLI_COMMAND_ARG_NAME_PREFIX "\"\r\n");
            } else {
                if(token_info->name != NULL) {
                    free(token_info->name);
                }

                size_t temp_cstr_len = furi_string_size(temp_str);
                token_info->name = malloc(temp_cstr_len + 1);
                furi_check(token_info->name != NULL);
                strlcpy(token_info->name, furi_string_get_cstr(temp_str), temp_cstr_len + 1);
                parsed = true;
            }
        } else if(furi_string_cmpi_str(temp_str, TOTP_CLI_COMMAND_ARG_ALGO_PREFIX) == 0) {
            if(!args_read_string_and_trim(args, temp_str)) {
                TOTP_CLI_PRINTF_ERROR(
                    "Missed value for argument \"" TOTP_CLI_COMMAND_ARG_ALGO_PREFIX "\"\r\n");
            } else if(!token_info_set_algo_from_str(token_info, temp_str)) {
                TOTP_CLI_PRINTF_ERROR(
                    "\"%s\" is incorrect value for argument \"" TOTP_CLI_COMMAND_ARG_ALGO_PREFIX
                    "\"\r\n",
                    furi_string_get_cstr(temp_str));
            } else {
                parsed = true;
            }
        } else if(furi_string_cmpi_str(temp_str, TOTP_CLI_COMMAND_ARG_DIGITS_PREFIX) == 0) {
            uint8_t digit_value;
            if(!args_read_uint8_and_trim(args, &digit_value)) {
                TOTP_CLI_PRINTF_ERROR(
                    "Missed or incorrect value for argument \"" TOTP_CLI_COMMAND_ARG_DIGITS_PREFIX
                    "\"\r\n");
            } else if(!token_info_set_digits_from_int(token_info, digit_value)) {
                TOTP_CLI_PRINTF_ERROR(
                    "\"%" PRIu8
                    "\" is incorrect value for argument \"" TOTP_CLI_COMMAND_ARG_DIGITS_PREFIX
                    "\"\r\n",
                    digit_value);
            } else {
                parsed = true;
            }
        } else if(furi_string_cmpi_str(temp_str, TOTP_CLI_COMMAND_ARG_DURATION_PREFIX) == 0) {
            uint8_t duration_value;
            if(!args_read_uint8_and_trim(args, &duration_value)) {
                TOTP_CLI_PRINTF_ERROR(
                    "Missed or incorrect value for argument \"" TOTP_CLI_COMMAND_ARG_DURATION_PREFIX
                    "\"\r\n");
            } else if(!token_info_set_duration_from_int(token_info, duration_value)) {
                TOTP_CLI_PRINTF_ERROR(
                    "\"%" PRIu8
                    "\" is incorrect value for argument \"" TOTP_CLI_COMMAND_ARG_DURATION_PREFIX
                    "\"\r\n",
                    duration_value);
            } else {
                parsed = true;
            }
        } else if(furi_string_cmpi_str(temp_str, TOTP_CLI_COMMAND_ARG_UNSECURE_PREFIX) == 0) {
            mask_user_input = false;
            parsed = true;
        } else if(furi_string_cmpi_str(temp_str, TOTP_CLI_COMMAND_UPDATE_ARG_SECRET_PREFIX) == 0) {
            update_token_secret = true;
            parsed = true;
        } else if(furi_string_cmpi_str(temp_str, TOTP_CLI_COMMAND_ARG_AUTOMATION_FEATURE_PREFIX) == 0) {
            if(!args_read_string_and_trim(args, temp_str)) {
                TOTP_CLI_PRINTF_ERROR(
                    "Missed value for argument \"" TOTP_CLI_COMMAND_ARG_AUTOMATION_FEATURE_PREFIX
                    "\"\r\n");
            } else if(!token_info_set_automation_feature_from_str(token_info, temp_str)) {
                TOTP_CLI_PRINTF_ERROR(
                    "\"%s\" is incorrect value for argument \"" TOTP_CLI_COMMAND_ARG_AUTOMATION_FEATURE_PREFIX
                    "\"\r\n",
                    furi_string_get_cstr(temp_str));
            } else {
                parsed = true;
            }
        } else {
            TOTP_CLI_PRINTF_ERROR("Unknown argument \"%s\"\r\n", furi_string_get_cstr(temp_str));
        }

        if(!parsed) {
            TOTP_CLI_PRINT_INVALID_ARGUMENTS();
            furi_string_free(temp_str);
            token_info_free(token_info);
            return;
        }
    }

    if(update_token_secret) {
        // Reading token secret
        furi_string_reset(temp_str);
        TOTP_CLI_PRINTF("Enter token secret and confirm with [ENTER]\r\n");
        if(!totp_cli_read_line(cli, temp_str, mask_user_input) ||
           !totp_cli_ensure_authenticated(plugin_state, cli)) {
            TOTP_CLI_DELETE_LAST_LINE();
            TOTP_CLI_PRINTF_INFO("Cancelled by user\r\n");
            furi_string_secure_free(temp_str);
            token_info_free(token_info);
            return;
        }

        TOTP_CLI_DELETE_LAST_LINE();

        if(token_info->token != NULL) {
            free(token_info->token);
        }

        if(!token_info_set_secret(
               token_info,
               furi_string_get_cstr(temp_str),
               furi_string_size(temp_str),
               plugin_state->iv)) {
            TOTP_CLI_PRINTF_ERROR("Token secret seems to be invalid and can not be parsed\r\n");
            furi_string_secure_free(temp_str);
            token_info_free(token_info);
            return;
        }
    }

    furi_string_secure_free(temp_str);

    bool load_generate_token_scene = false;
    if(plugin_state->current_scene == TotpSceneGenerateToken) {
        totp_scene_director_activate_scene(plugin_state, TotpSceneNone, NULL);
        load_generate_token_scene = true;
    }

    list_item->data = token_info;

    if(totp_full_save_config_file(plugin_state) == TotpConfigFileUpdateSuccess) {
        TOTP_CLI_PRINTF_SUCCESS(
            "Token \"%s\" has been successfully updated\r\n", token_info->name);
        token_info_free(existing_token_info);
    } else {
        TOTP_CLI_PRINT_ERROR_UPDATING_CONFIG_FILE();
        list_item->data = existing_token_info;
        token_info_free(token_info);
    }

    if(load_generate_token_scene) {
        totp_scene_director_activate_scene(plugin_state, TotpSceneGenerateToken, NULL);
    }
}