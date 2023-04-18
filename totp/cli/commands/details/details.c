#include "details.h"
#include <stdlib.h>
#include <lib/toolbox/args.h>
#include "../../../types/token_info.h"
#include "../../../services/config/constants.h"
#include "../../cli_helpers.h"
#include "../../common_command_arguments.h"

#define TOTP_CLI_PRINTF_AUTOMATION_FEATURE(description, header_printed) \
    do {                                                                \
        TOTP_CLI_PRINTF(                                                \
            "| %-20s | %-28.28s |\r\n",                                 \
            header_printed ? "" : "Automation features",                \
            description);                                               \
        header_printed = true;                                          \
    } while(false)

static void print_automation_features(const TokenInfo* token_info) {
    if(token_info->automation_features == TOKEN_AUTOMATION_FEATURE_NONE) {
        TOTP_CLI_PRINTF("| %-20s | %-28.28s |\r\n", "Automation features", "None");
        return;
    }

    bool header_printed = false;
    if(token_info->automation_features & TOKEN_AUTOMATION_FEATURE_ENTER_AT_THE_END) {
        TOTP_CLI_PRINTF_AUTOMATION_FEATURE("Type <Enter> key at the end", header_printed);
    }

    if(token_info->automation_features & TOKEN_AUTOMATION_FEATURE_TAB_AT_THE_END) {
        TOTP_CLI_PRINTF_AUTOMATION_FEATURE("Type <Tab> key at the end", header_printed);
    }

    if(token_info->automation_features & TOKEN_AUTOMATION_FEATURE_TYPE_SLOWER) {
        TOTP_CLI_PRINTF_AUTOMATION_FEATURE("Type slower", header_printed);
    }
}

void totp_cli_command_details_docopt_commands() {
    TOTP_CLI_PRINTF("  " TOTP_CLI_COMMAND_DETAILS ", " TOTP_CLI_COMMAND_DETAILS_ALT
                    "      Displays token details\r\n");
}

void totp_cli_command_details_docopt_usage() {
    TOTP_CLI_PRINTF("  " TOTP_CLI_COMMAND_NAME " " DOCOPT_REQUIRED(
        TOTP_CLI_COMMAND_DETAILS
        " | " TOTP_CLI_COMMAND_DETAILS_ALT) " " DOCOPT_ARGUMENT(TOTP_CLI_COMMAND_ARG_INDEX) "\r\n");
}

void totp_cli_command_details_handle(PluginState* plugin_state, FuriString* args, Cli* cli) {
    if(!totp_cli_ensure_authenticated(plugin_state, cli)) {
        return;
    }

    int token_number;
    TokenInfoIteratorContext* iterator_context = plugin_state->config_file_context->token_info_iterator_context;
    if(!args_read_int_and_trim(args, &token_number) || token_number <= 0 ||
       (size_t)token_number > iterator_context->total_count) {
        TOTP_CLI_PRINT_INVALID_ARGUMENTS();
        return;
    }


    size_t original_token_index = iterator_context->current_index;
    iterator_context->current_index = token_number - 1;
    if (totp_token_info_iterator_load_current_token_info(iterator_context)) {
        TokenInfo* token_info = iterator_context->current_token;

        TOTP_CLI_PRINTF("+----------------------+------------------------------+\r\n");
        TOTP_CLI_PRINTF("| %-20s | %-28s |\r\n", "Property", "Value");
        TOTP_CLI_PRINTF("+----------------------+------------------------------+\r\n");
        TOTP_CLI_PRINTF("| %-20s | %-28d |\r\n", "Index", token_number);
        TOTP_CLI_PRINTF("| %-20s | %-28.28s |\r\n", "Name", furi_string_get_cstr(token_info->name_n));
        TOTP_CLI_PRINTF(
            "| %-20s | %-28s |\r\n", "Hashing algorithm", token_info_get_algo_as_cstr(token_info));
        TOTP_CLI_PRINTF("| %-20s | %-28" PRIu8 " |\r\n", "Number of digits", token_info->digits);
        TOTP_CLI_PRINTF(
            "| %-20s | %" PRIu8 " sec.%-21s |\r\n", "Token lifetime", token_info->duration, " ");
        print_automation_features(token_info);
        TOTP_CLI_PRINTF("+----------------------+------------------------------+\r\n");
    } else {
        TOTP_CLI_PRINT_ERROR_LOADING_TOKEN_INFO();
    }

    iterator_context->current_index = original_token_index;
    totp_token_info_iterator_load_current_token_info(iterator_context);
}