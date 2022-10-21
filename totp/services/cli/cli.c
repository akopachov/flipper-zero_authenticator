#include "../../types/plugin_state.h"
#include "../../types/token_info.h"
#include "../../../../lib/toolbox/args.h"
#include "../config/config.h"

static void cli_command_totp_print_usage() {
    printf("Usage:\r\n");
    printf("totp <cmd> <args>\r\n");
    printf("Cmd list:\r\n");
    printf("\tlist - (NOT IMPLEMENTED) list all tokens\r\n");
    printf("\tdel <NAME> - (NOT IMPLEMENTED) delete token\r\n");
    printf("\t\t<NAME>   - token name\r\n");
    printf("\tadd <NAME> <ALGO> <DIGITS> <SECRET>\t - add new token\r\n");
    printf("\t\t<NAME>   - token name\r\n");
    printf("\t\t<ALGO>   - token algo, one of: sha1,sha256,sha512; default: sha1\r\n");
    printf("\t\t<DIGITS> - number of digits to generate, one of: 6,8; default: 6\r\n");
    printf("\t\t<SECRET> - Base32 token secret\r\n");
    printf("\r\n");
}

static TokenHashAlgo furi_string_to_token_hash_algo(FuriString* str) {
    if(!furi_string_cmp_str(str, "sha1")) {
        return SHA1;
    } else if(!furi_string_cmp_str(str, "sha256")) {
        return SHA256;
    } else if(!furi_string_cmp_str(str, "sha512")) {
        return SHA512;
    } else {
        printf(">>> Warning - Could not parse algo, using SHA1 as default\r\n");
        return SHA1;
    }
}

static TokenDigitsCount int_to_token_digits_count(int value) {
    if(value == 6) {
        return TOTP_6_DIGITS;
    } else if(value == 8) {
        return TOTP_8_DIGITS;
    } else {
        printf(">>> Warning - Could not parse number of digits, using default (6)\r\n");
        return TOTP_6_DIGITS;
    }
}

static int cli_command_totp_add_parse_args(
    FuriString* args,
    PluginState* plugin_state,
    TokenInfo* tokenInfo) {
    int errors = 0;
    tokenInfo->name = malloc(args_get_first_word_length(args) + 1);

    FuriString* tokenName;
    tokenName = furi_string_alloc();
    if(args_read_string_and_trim(args, tokenName)) {
        strcpy(tokenInfo->name, furi_string_get_cstr(tokenName));
    } else {
        printf(">>> Error parsing <name> arg\r\n");
        errors += 1;
    }
    furi_string_free(tokenName);

    FuriString* tokenAlgo;
    tokenAlgo = furi_string_alloc();
    if(args_read_string_and_trim(args, tokenAlgo)) {
        tokenInfo->algo = furi_string_to_token_hash_algo(tokenAlgo);
    } else {
        printf(">>> Error parsing <algo> arg\r\n");
        errors += 1;
    }
    furi_string_free(tokenAlgo);

    int tokenDigits = 6;
    if(args_read_int_and_trim(args, &tokenDigits)) {
        tokenInfo->digits = int_to_token_digits_count(tokenDigits);
    } else {
        printf(">>> Error parsing <digits> arg\r\n");
        errors += 1;
    }

    FuriString* tokenSecret;
    tokenSecret = furi_string_alloc();
    if(args_read_string_and_trim(args, tokenSecret)) {
        bool token_secret_set = token_info_set_secret(
            tokenInfo,
            furi_string_get_cstr(tokenSecret),
            furi_string_size(tokenSecret),
            &plugin_state->iv[0]);

        if(!token_secret_set) {
            printf(">>> Error - Token secret is invalid\r\n");
            errors += 1;
        }

    } else {
        printf(">>> Error parsing <secret> arg\r\n");
        errors += 1;
    }
    furi_string_free(tokenSecret);

    return errors;
}

static bool cli_command_totp_add(Cli* cli, FuriString* args, PluginState* plugin_state) {
    UNUSED(cli);

    bool result = true;
    TokenInfo* tokenInfo = token_info_alloc();

    if(cli_command_totp_add_parse_args(args, plugin_state, tokenInfo) == 0) {
        if(plugin_state->tokens_list == NULL) {
            plugin_state->tokens_list = list_init_head(tokenInfo);
        } else {
            list_add(plugin_state->tokens_list, tokenInfo);
        }
        plugin_state->tokens_count++;

        totp_config_file_save_new_token(tokenInfo);

        printf("Added token '%s'\r\n", tokenInfo->name);
    } else {
        printf(">>> Error adding token - see errors above");
        result = false;
    }

    token_info_free(tokenInfo);
    return result;
}

void cli_command_totp(Cli* cli, FuriString* args, void* plugin_state) {
    FuriString* cmd;
    cmd = furi_string_alloc();
    args_read_string_and_trim(args, cmd);
    do {
        if(furi_string_cmp_str(cmd, "help") == 0) {
            cli_command_totp_print_usage();
            break;
        }

        if(furi_string_cmp_str(cmd, "add") == 0) {
            cli_command_totp_add(cli, args, plugin_state);
            break;
        }

        if(furi_string_cmp_str(cmd, "list") == 0) {
            cli_command_totp_print_usage();
            break;
        }

        if(furi_string_cmp_str(cmd, "del") == 0) {
            cli_command_totp_print_usage();
            break;
        }

        cli_command_totp_print_usage();
    } while(false);

    furi_string_free(cmd);
}
