#include "add.h"
#include <stdlib.h>
#include <lib/toolbox/args.h>
#include "../../../types/token_info.h"
#include "../../../services/config/config.h"
#include "../../../services/convert/convert.h"
#include "../../cli_helpers.h"
#include "../../../ui/scene_director.h"
#include "../../common_command_arguments.h"

void totp_cli_command_add_docopt_commands() {
    TOTP_CLI_PRINTF("  " TOTP_CLI_COMMAND_ADD ", " TOTP_CLI_COMMAND_ADD_ALT
                    ", " TOTP_CLI_COMMAND_ADD_ALT2 "     Add new token\r\n");
}

void totp_cli_command_add_docopt_usage() {
    TOTP_CLI_PRINTF(
        "  " TOTP_CLI_COMMAND_NAME
        " " DOCOPT_REQUIRED(TOTP_CLI_COMMAND_ADD " | " TOTP_CLI_COMMAND_ADD_ALT " | " TOTP_CLI_COMMAND_ADD_ALT2) " " DOCOPT_ARGUMENT(TOTP_CLI_COMMAND_ARG_NAME) " " DOCOPT_OPTIONAL(DOCOPT_OPTION(TOTP_CLI_COMMAND_ARG_ALGO_PREFIX, DOCOPT_ARGUMENT(TOTP_CLI_COMMAND_ARG_ALGO))) " " DOCOPT_OPTIONAL(DOCOPT_OPTION(TOTP_CLI_COMMAND_ARG_SECRET_ENCODING_PREFIX, DOCOPT_ARGUMENT(TOTP_CLI_COMMAND_ARG_SECRET_ENCODING))) " " DOCOPT_OPTIONAL(
            DOCOPT_OPTION(
                TOTP_CLI_COMMAND_ARG_DIGITS_PREFIX,
                DOCOPT_ARGUMENT(
                    TOTP_CLI_COMMAND_ARG_DIGITS))) " " DOCOPT_OPTIONAL(DOCOPT_OPTION(TOTP_CLI_COMMAND_ARG_DURATION_PREFIX, DOCOPT_ARGUMENT(TOTP_CLI_COMMAND_ARG_DURATION))) " " DOCOPT_OPTIONAL(DOCOPT_SWITCH(TOTP_CLI_COMMAND_ARG_UNSECURE_PREFIX)) " " DOCOPT_MULTIPLE(DOCOPT_OPTIONAL(DOCOPT_OPTION(TOTP_CLI_COMMAND_ARG_AUTOMATION_FEATURE_PREFIX, DOCOPT_ARGUMENT(TOTP_CLI_COMMAND_ARG_AUTOMATION_FEATURE)))) "\r\n");
}

void totp_cli_command_add_docopt_arguments() {
    TOTP_CLI_PRINTF("  " TOTP_CLI_COMMAND_ARG_NAME "          Token name\r\n");
}

void totp_cli_command_add_docopt_options() {
    TOTP_CLI_PRINTF("  " DOCOPT_OPTION(
        TOTP_CLI_COMMAND_ARG_ALGO_PREFIX,
        DOCOPT_ARGUMENT(
            TOTP_CLI_COMMAND_ARG_ALGO)) "      Token hashing algorithm. Must be one of: " TOTP_TOKEN_ALGO_SHA1_NAME
                                        ", " TOTP_TOKEN_ALGO_SHA256_NAME
                                        ", " TOTP_TOKEN_ALGO_SHA512_NAME
                                        ", " TOTP_TOKEN_ALGO_STEAM_NAME
                                        " " DOCOPT_DEFAULT(TOTP_TOKEN_ALGO_SHA1_NAME) "\r\n");
    TOTP_CLI_PRINTF("  " DOCOPT_OPTION(
        TOTP_CLI_COMMAND_ARG_DIGITS_PREFIX,
        DOCOPT_ARGUMENT(
            TOTP_CLI_COMMAND_ARG_DIGITS)) "    Number of digits to generate, one of: 5, 6, 8 " DOCOPT_DEFAULT("6") "\r\n");

    TOTP_CLI_PRINTF("  " DOCOPT_OPTION(
        TOTP_CLI_COMMAND_ARG_SECRET_ENCODING_PREFIX,
        DOCOPT_ARGUMENT(
            TOTP_CLI_COMMAND_ARG_SECRET_ENCODING)) "  Token secret encoding, one of " PLAIN_TOKEN_ENCODING_BASE32_NAME
                                                   ", " PLAIN_TOKEN_ENCODING_BASE64_NAME
                                                   " " DOCOPT_DEFAULT(
                                                       PLAIN_TOKEN_ENCODING_BASE32_NAME) "\r\n");

    TOTP_CLI_PRINTF("  " DOCOPT_OPTION(
        TOTP_CLI_COMMAND_ARG_DURATION_PREFIX,
        DOCOPT_ARGUMENT(
            TOTP_CLI_COMMAND_ARG_DURATION)) "  Token lifetime duration in seconds, between: 15 and 255 " DOCOPT_DEFAULT("30") "\r\n");
    TOTP_CLI_PRINTF("  " DOCOPT_SWITCH(
        TOTP_CLI_COMMAND_ARG_UNSECURE_PREFIX) "             Show console user input as-is without masking\r\n");
    TOTP_CLI_PRINTF("  " DOCOPT_OPTION(
        TOTP_CLI_COMMAND_ARG_AUTOMATION_FEATURE_PREFIX,
        DOCOPT_ARGUMENT(
            TOTP_CLI_COMMAND_ARG_AUTOMATION_FEATURE)) "   Token automation features to be enabled. Must be one of: " TOTP_TOKEN_AUTOMATION_FEATURE_NONE_NAME
                                                      ", " TOTP_TOKEN_AUTOMATION_FEATURE_ENTER_AT_THE_END_NAME
                                                      ", " TOTP_TOKEN_AUTOMATION_FEATURE_TAB_AT_THE_END_NAME
                                                      " " DOCOPT_DEFAULT(
                                                          TOTP_TOKEN_AUTOMATION_FEATURE_NONE_NAME) "\r\n");
    TOTP_CLI_PRINTF("                 # " TOTP_TOKEN_AUTOMATION_FEATURE_NONE_NAME
                    " - No features\r\n");
    TOTP_CLI_PRINTF("                 # " TOTP_TOKEN_AUTOMATION_FEATURE_ENTER_AT_THE_END_NAME
                    " - Type <Enter> key at the end of token input automation\r\n");
    TOTP_CLI_PRINTF("                 # " TOTP_TOKEN_AUTOMATION_FEATURE_TAB_AT_THE_END_NAME
                    " - Type <Tab> key at the end of token input automation\r\n");
    TOTP_CLI_PRINTF("                 # " TOTP_TOKEN_AUTOMATION_FEATURE_TYPE_SLOWER_NAME
                    " - Type slower\r\n");
}

void totp_cli_command_add_handle(PluginState* plugin_state, FuriString* args, Cli* cli) {
    if(!totp_cli_ensure_authenticated(plugin_state, cli)) {
        return;
    }

    TokenInfoIteratorContext* iterator_context =
        plugin_state->config_file_context->token_info_iterator_context;
    TokenInfo* token_info = iterator_context->current_token;

    TOTP_CLI_LOCK_UI(plugin_state);

    token_info_set_defaults(token_info);

    // Reading token name
    if(!args_read_probably_quoted_string_and_trim(args, token_info->name)) {
        totp_cli_print_invalid_arguments();
        totp_token_info_iterator_load_current_token_info(iterator_context);
        TOTP_CLI_UNLOCK_UI(plugin_state);
        return;
    }

    FuriString* temp_str = furi_string_alloc();

    // Read optional arguments
    bool mask_user_input = true;
    PlainTokenSecretEncoding token_secret_encoding = PLAIN_TOKEN_ENCODING_BASE32;
    while(args_read_string_and_trim(args, temp_str)) {
        bool parsed = false;
        if(!totp_cli_try_read_algo(token_info, temp_str, args, &parsed) &&
           !totp_cli_try_read_digits(token_info, temp_str, args, &parsed) &&
           !totp_cli_try_read_duration(token_info, temp_str, args, &parsed) &&
           !totp_cli_try_read_unsecure_flag(temp_str, &parsed, &mask_user_input) &&
           !totp_cli_try_read_automation_features(token_info, temp_str, args, &parsed) &&
           !totp_cli_try_read_plain_token_secret_encoding(
               temp_str, args, &parsed, &token_secret_encoding)) {
            totp_cli_printf_unknown_argument(temp_str);
        }

        if(!parsed) {
            totp_cli_print_invalid_arguments();
            furi_string_free(temp_str);
            totp_token_info_iterator_load_current_token_info(iterator_context);
            TOTP_CLI_UNLOCK_UI(plugin_state);
            return;
        }
    }

    // Reading token secret
    furi_string_reset(temp_str);
    TOTP_CLI_PRINTF("Enter token secret and confirm with [ENTER]\r\n");
    if(!totp_cli_read_line(cli, temp_str, mask_user_input)) {
        totp_cli_delete_last_line();
        TOTP_CLI_PRINTF_INFO("Cancelled by user\r\n");
        furi_string_secure_free(temp_str);
        totp_token_info_iterator_load_current_token_info(iterator_context);
        TOTP_CLI_UNLOCK_UI(plugin_state);
        return;
    }

    totp_cli_delete_last_line();

    bool secret_set = token_info_set_secret(
        token_info,
        furi_string_get_cstr(temp_str),
        furi_string_size(temp_str),
        token_secret_encoding,
        plugin_state->iv);

    furi_string_secure_free(temp_str);

    if(secret_set) {
        size_t previous_index = iterator_context->current_index;
        iterator_context->current_index = iterator_context->total_count;
        totp_cli_print_processing();
        if(totp_token_info_iterator_save_current_token_info_changes(iterator_context)) {
            totp_cli_delete_last_line();
            TOTP_CLI_PRINTF_SUCCESS(
                "Token \"%s\" has been successfully added\r\n",
                furi_string_get_cstr(token_info->name));
        } else {
            totp_cli_delete_last_line();
            totp_cli_print_error_updating_config_file();
            iterator_context->current_index = previous_index;
        }
    } else {
        TOTP_CLI_PRINTF_ERROR("Token secret seems to be invalid and can not be parsed\r\n");
    }

    totp_token_info_iterator_load_current_token_info(iterator_context);
    TOTP_CLI_UNLOCK_UI(plugin_state);
}