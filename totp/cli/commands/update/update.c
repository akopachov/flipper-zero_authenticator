#include "update.h"
#include <stdlib.h>
#include <lib/toolbox/args.h>
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
        " " DOCOPT_REQUIRED(TOTP_CLI_COMMAND_UPDATE) " " DOCOPT_ARGUMENT(TOTP_CLI_COMMAND_ARG_INDEX) " " DOCOPT_OPTIONAL(DOCOPT_OPTION(TOTP_CLI_COMMAND_ARG_ALGO_PREFIX, DOCOPT_ARGUMENT(TOTP_CLI_COMMAND_ARG_ALGO))) " " DOCOPT_OPTIONAL(DOCOPT_OPTION(TOTP_CLI_COMMAND_ARG_SECRET_ENCODING_PREFIX, DOCOPT_ARGUMENT(TOTP_CLI_COMMAND_ARG_SECRET_ENCODING))) " " DOCOPT_OPTIONAL(DOCOPT_OPTION(TOTP_CLI_COMMAND_ARG_NAME_PREFIX, DOCOPT_ARGUMENT(TOTP_CLI_COMMAND_ARG_NAME))) " " DOCOPT_OPTIONAL(
            DOCOPT_OPTION(
                TOTP_CLI_COMMAND_ARG_DIGITS_PREFIX,
                DOCOPT_ARGUMENT(
                    TOTP_CLI_COMMAND_ARG_DIGITS))) " " DOCOPT_OPTIONAL(DOCOPT_OPTION(TOTP_CLI_COMMAND_ARG_DURATION_PREFIX, DOCOPT_ARGUMENT(TOTP_CLI_COMMAND_ARG_DURATION))) " " DOCOPT_OPTIONAL(DOCOPT_SWITCH(TOTP_CLI_COMMAND_ARG_UNSECURE_PREFIX)) " " DOCOPT_OPTIONAL(DOCOPT_SWITCH(TOTP_CLI_COMMAND_UPDATE_ARG_SECRET_PREFIX)) " " DOCOPT_MULTIPLE(DOCOPT_OPTIONAL(DOCOPT_OPTION(TOTP_CLI_COMMAND_ARG_AUTOMATION_FEATURE_PREFIX, DOCOPT_ARGUMENT(TOTP_CLI_COMMAND_ARG_AUTOMATION_FEATURE)))) "\r\n");
}

void totp_cli_command_update_docopt_options() {
    TOTP_CLI_PRINTF("  " DOCOPT_OPTION(
        TOTP_CLI_COMMAND_ARG_NAME_PREFIX,
        DOCOPT_ARGUMENT(TOTP_CLI_COMMAND_ARG_NAME)) "      Token name\r\n");

    TOTP_CLI_PRINTF("  " DOCOPT_SWITCH(
        TOTP_CLI_COMMAND_UPDATE_ARG_SECRET_PREFIX) "             Update token secret\r\n");
}

static bool
    totp_cli_try_read_name(TokenInfo* token_info, FuriString* arg, FuriString* args, bool* parsed) {
    if(furi_string_cmpi_str(arg, TOTP_CLI_COMMAND_ARG_NAME_PREFIX) == 0) {
        if(!args_read_probably_quoted_string_and_trim(args, token_info->name) ||
           furi_string_empty(token_info->name)) {
            totp_cli_printf_missed_argument_value(TOTP_CLI_COMMAND_ARG_NAME_PREFIX);
        } else {
            *parsed = true;
        }

        return true;
    }

    return false;
}

static bool totp_cli_try_read_change_secret_flag(const FuriString* arg, bool* parsed, bool* flag) {
    if(furi_string_cmpi_str(arg, TOTP_CLI_COMMAND_UPDATE_ARG_SECRET_PREFIX) == 0) {
        *flag = true;
        *parsed = true;
        return true;
    }

    return false;
}

void totp_cli_command_update_handle(PluginState* plugin_state, FuriString* args, Cli* cli) {
    if(!totp_cli_ensure_authenticated(plugin_state, cli)) {
        return;
    }

    FuriString* temp_str = furi_string_alloc();

    TokenInfoIteratorContext* iterator_context =
        plugin_state->config_file_context->token_info_iterator_context;

    int token_number;
    if(!args_read_int_and_trim(args, &token_number) || token_number <= 0 ||
       (size_t)token_number > iterator_context->total_count) {
        totp_cli_print_invalid_arguments();
        return;
    }

    TOTP_CLI_LOCK_UI(plugin_state);

    size_t previous_index = iterator_context->current_index;
    iterator_context->current_index = token_number - 1;
    totp_token_info_iterator_load_current_token_info(iterator_context);
    TokenInfo* token_info = iterator_context->current_token;

    // Read optional arguments
    bool mask_user_input = true;
    bool update_token_secret = false;
    PlainTokenSecretEncoding token_secret_encoding = PLAIN_TOKEN_ENCODING_BASE32;
    while(args_read_string_and_trim(args, temp_str)) {
        bool parsed = false;
        if(!totp_cli_try_read_name(token_info, temp_str, args, &parsed) &&
           !totp_cli_try_read_algo(token_info, temp_str, args, &parsed) &&
           !totp_cli_try_read_digits(token_info, temp_str, args, &parsed) &&
           !totp_cli_try_read_duration(token_info, temp_str, args, &parsed) &&
           !totp_cli_try_read_unsecure_flag(temp_str, &parsed, &mask_user_input) &&
           !totp_cli_try_read_change_secret_flag(temp_str, &parsed, &update_token_secret) &&
           !totp_cli_try_read_automation_features(token_info, temp_str, args, &parsed) &&
           !totp_cli_try_read_plain_token_secret_encoding(
               temp_str, args, &parsed, &token_secret_encoding)) {
            totp_cli_printf_unknown_argument(temp_str);
        }

        if(!parsed) {
            totp_cli_print_invalid_arguments();
            furi_string_free(temp_str);
            iterator_context->current_index = previous_index;
            totp_token_info_iterator_load_current_token_info(iterator_context);
            return;
        }
    }

    bool token_secret_read = false;
    if(update_token_secret) {
        // Reading token secret
        furi_string_reset(temp_str);
        TOTP_CLI_PRINTF("Enter token secret and confirm with [ENTER]\r\n");
        token_secret_read = totp_cli_read_line(cli, temp_str, mask_user_input);
        totp_cli_delete_last_line();
        if(!token_secret_read) {
            TOTP_CLI_PRINTF_INFO("Cancelled by user\r\n");
        }
    }

    bool token_secret_set = false;
    if(update_token_secret && token_secret_read) {
        token_secret_set = token_info_set_secret(
            token_info,
            furi_string_get_cstr(temp_str),
            furi_string_size(temp_str),
            token_secret_encoding,
            plugin_state->iv);
        if(!token_secret_set) {
            TOTP_CLI_PRINTF_ERROR("Token secret seems to be invalid and can not be parsed\r\n");
        }
    }

    furi_string_secure_free(temp_str);

    if(!update_token_secret || (token_secret_read && token_secret_set)) {
        totp_cli_print_processing();
        if(totp_token_info_iterator_save_current_token_info_changes(iterator_context)) {
            totp_cli_delete_last_line();
            TOTP_CLI_PRINTF_SUCCESS(
                "Token \"%s\" has been successfully updated\r\n",
                furi_string_get_cstr(token_info->name));
        } else {
            totp_cli_delete_last_line();
            totp_cli_print_error_updating_config_file();
        }
    }

    iterator_context->current_index = previous_index;
    totp_token_info_iterator_load_current_token_info(iterator_context);
    TOTP_CLI_UNLOCK_UI(plugin_state);
}