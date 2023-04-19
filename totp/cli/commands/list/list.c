#include "list.h"
#include <stdlib.h>
#include "../../../types/token_info.h"
#include "../../../services/config/constants.h"
#include "../../../ui/scene_director.h"
#include "../../cli_helpers.h"

void totp_cli_command_list_docopt_commands() {
    TOTP_CLI_PRINTF("  " TOTP_CLI_COMMAND_LIST ", " TOTP_CLI_COMMAND_LIST_ALT
                    "         List all available tokens\r\n");
}

void totp_cli_command_list_docopt_usage() {
    TOTP_CLI_PRINTF("  " TOTP_CLI_COMMAND_NAME " " DOCOPT_REQUIRED(
        TOTP_CLI_COMMAND_LIST " | " TOTP_CLI_COMMAND_LIST_ALT) "\r\n");
}

void totp_cli_command_list_handle(PluginState* plugin_state, Cli* cli) {
    if(!totp_cli_ensure_authenticated(plugin_state, cli)) {
        return;
    }

    TokenInfoIteratorContext* iterator_context = plugin_state->config_file_context->token_info_iterator_context;
    if(iterator_context->total_count <= 0) {
        TOTP_CLI_PRINTF("There are no tokens");
        return;
    }

    TOTP_CLI_LOCK_UI(plugin_state);

    size_t original_index = iterator_context->current_index;

    TOTP_CLI_PRINTF("+-----+---------------------------+--------+----+-----+\r\n");
    TOTP_CLI_PRINTF("| %-3s | %-25s | %-6s | %-s | %-s |\r\n", "#", "Name", "Algo", "Ln", "Dur");
    TOTP_CLI_PRINTF("+-----+---------------------------+--------+----+-----+\r\n");
    for (size_t i = 0; i < iterator_context->total_count; i++) {
        iterator_context->current_index = i;
        totp_token_info_iterator_load_current_token_info(iterator_context);
        TokenInfo* token_info = iterator_context->current_token;
        TOTP_CLI_PRINTF(
            "| %-3" PRIu16 " | %-25.25s | %-6s | %-2" PRIu8 " | %-3" PRIu8 " |\r\n",
            i + 1,
            furi_string_get_cstr(token_info->name_n),
            token_info_get_algo_as_cstr(token_info),
            token_info->digits,
            token_info->duration);
    }

    TOTP_CLI_PRINTF("+-----+---------------------------+--------+----+-----+\r\n");

    iterator_context->current_index = original_index;
    totp_token_info_iterator_load_current_token_info(iterator_context);

    TOTP_CLI_UNLOCK_UI(plugin_state);
}