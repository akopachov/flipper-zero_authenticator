#include "move.h"

#include <stdlib.h>
#include <lib/toolbox/args.h>
#include "../../../types/token_info.h"
#include "../../../services/config/config.h"
#include "../../cli_helpers.h"
#include "../../../ui/scene_director.h"
#include "../../common_command_arguments.h"

#define TOTP_CLI_COMMAND_MOVE_ARG_NEW_INDEX "new_index"

void totp_cli_command_move_docopt_commands() {
    TOTP_CLI_PRINTF("  " TOTP_CLI_COMMAND_MOVE ", " TOTP_CLI_COMMAND_MOVE_ALT
                    "         Move token\r\n");
}

void totp_cli_command_move_docopt_usage() {
    TOTP_CLI_PRINTF(
        "  " TOTP_CLI_COMMAND_NAME
        " " DOCOPT_REQUIRED(TOTP_CLI_COMMAND_MOVE " | " TOTP_CLI_COMMAND_MOVE_ALT) " " DOCOPT_ARGUMENT(
            TOTP_CLI_COMMAND_ARG_INDEX) " " DOCOPT_ARGUMENT(TOTP_CLI_COMMAND_MOVE_ARG_NEW_INDEX) "\r\n");
}

void totp_cli_command_move_docopt_arguments() {
    TOTP_CLI_PRINTF("  " TOTP_CLI_COMMAND_MOVE_ARG_NEW_INDEX
                    "     New token index in the list\r\n");
}

void totp_cli_command_move_handle(PluginState* plugin_state, FuriString* args, Cli* cli) {
    if(!totp_cli_ensure_authenticated(plugin_state, cli)) {
        return;
    }

    int token_number;
    if(!args_read_int_and_trim(args, &token_number) || token_number < 1 ||
       (size_t)token_number > plugin_state->config_file_context->token_info_iterator_context->total_count) {
        TOTP_CLI_PRINT_INVALID_ARGUMENTS();
        return;
    }

    int new_token_number = 0;

    if(!args_read_int_and_trim(args, &new_token_number) || new_token_number < 1 ||
       (size_t)new_token_number > plugin_state->config_file_context->token_info_iterator_context->total_count) {
        TOTP_CLI_PRINT_INVALID_ARGUMENTS();
        return;
    }

    if (token_number == new_token_number) {
        TOTP_CLI_PRINTF_ERROR("New token number matches current token number\r\n");
        return;
    }

    TOTP_CLI_LOCK_UI(plugin_state);

    size_t token_index = token_number - 1;
    size_t new_token_index = new_token_number - 1;

    TokenInfoIteratorContext* iterator_context = plugin_state->config_file_context->token_info_iterator_context;
    size_t original_token_index = iterator_context->current_index;

    iterator_context->current_index = token_index;

    if (totp_token_info_iterator_load_current_token_info(iterator_context) &&
        totp_token_info_iterator_move_current_token_info(iterator_context, new_token_index)) {
        TOTP_CLI_PRINTF_SUCCESS(
            "Token \"%s\" has been successfully updated\r\n", furi_string_get_cstr(iterator_context->current_token->name_n));
    } else {
        TOTP_CLI_PRINT_ERROR_UPDATING_CONFIG_FILE();
    }

    iterator_context->current_index = original_token_index;
    totp_token_info_iterator_load_current_token_info(iterator_context);

    TOTP_CLI_UNLOCK_UI(plugin_state);
}