#include "token_info_iterator.h"

#include <flipper_format/flipper_format_i.h>
#include <flipper_format/flipper_format_stream.h>
#include "../../types/common.h"

static bool flipper_format_seek_to_siblinig_token_start(Stream* stream, StreamDirection direction) {
    char buffer[sizeof(TOTP_CONFIG_KEY_TOKEN_NAME) + 1];
    bool found = false;
    while (!found) {
        if (!stream_seek_to_char(stream, '\n', direction)) {
            break;
        }

        if (!stream_read(stream, (uint8_t *)&buffer[0], sizeof(buffer))) {
            break;
        }

        stream_seek(stream, -sizeof(buffer), StreamOffsetFromCurrent);

        if (strncmp(buffer, ("\n" TOTP_CONFIG_KEY_TOKEN_NAME ":"), sizeof(buffer)) == 0) {
            found = true;
        }
    }

    return found;
}

static bool seek_to_token(size_t token_index, TokenInfoIteratorContext* context) {
    furi_check(context != NULL && context->config_file != NULL);
    if (token_index >= context->total_count) {
        return false;
    }

    Stream* stream = flipper_format_get_raw_stream(context->config_file);
    long token_index_diff;
    StreamDirection direction;
    bool edge_case;
    if (token_index == 0) {
        context->last_seek_offset = 0;
        context->last_seek_index = 0;
        token_index_diff = 0;
        direction = StreamDirectionForward;
        edge_case = true;
    } else if (token_index >= context->total_count - 1) {
        context->last_seek_offset = stream_size(stream);
        context->last_seek_index = context->total_count - 1;
        token_index_diff = 0;
        direction = StreamDirectionBackward;
        edge_case = true;
    } else {
        token_index_diff = (long)token_index - (long)context->last_seek_index;
        direction = token_index_diff >= 0 ? StreamDirectionForward : StreamDirectionBackward;
        edge_case = false;
    }

    FURI_LOG_D(LOGGING_TAG, "SEEKINIG TO: %u", context->last_seek_offset);
    stream_seek(stream, context->last_seek_offset, StreamOffsetFromStart);

    if (token_index_diff != 0 || edge_case) {    
        long i = 0;
        long i_inc = token_index_diff >= 0 ? 1 : -1;
        FURI_LOG_D(LOGGING_TAG, "Need to move by %ld tokens", token_index_diff);
        do {
            if(!flipper_format_seek_to_siblinig_token_start(stream, direction)) {
                break;
            }

            i += i_inc;
        } while ((i_inc > 0 && i < token_index_diff) || (i_inc < 0 && i > token_index_diff));

        if ((i_inc > 0 && i < token_index_diff) || (i_inc < 0 && i > token_index_diff)) {
            context->last_seek_offset = 0;
            FURI_LOG_D(LOGGING_TAG, "Was not able to move");
            return false;
        }

        context->last_seek_offset = stream_tell(stream);
        context->last_seek_index = token_index;
        

        FURI_LOG_D(LOGGING_TAG, "Moved to %u", context->last_seek_offset);
    }

    return true;
}

TokenInfoIteratorContext* totp_token_info_iterator_alloc(FlipperFormat* config_file, uint8_t* iv) {
    Stream* stream = flipper_format_get_raw_stream(config_file);
    stream_rewind(stream);
    size_t tokens_count = 0;
    while(true) {
        if(!flipper_format_seek_to_siblinig_token_start(stream, StreamDirectionForward)) {
            break;
        }

        tokens_count++;
    }

    TokenInfoIteratorContext* context = malloc(sizeof(TokenInfoIteratorContext));
    furi_check(context != NULL);

    context->total_count = tokens_count;
    context->current_token = token_info_alloc();
    context->config_file = config_file;
    context->iv = iv;
    return context;
}

void totp_token_info_iterator_free(TokenInfoIteratorContext* context) {
    if (context == NULL) return;
    token_info_free(context->current_token);
    free(context);
}

bool totp_token_info_iterator_remove_current_token_info(TokenInfoIteratorContext* context) {
    if (!seek_to_token(context->current_index, context)) {
        return false;
    }

    if (!flipper_format_delete_key(context->config_file, TOTP_CONFIG_KEY_TOKEN_NAME) ||
        !flipper_format_delete_key(context->config_file, TOTP_CONFIG_KEY_TOKEN_SECRET) ||
        !flipper_format_delete_key(context->config_file, TOTP_CONFIG_KEY_TOKEN_ALGO) ||
        !flipper_format_delete_key(context->config_file, TOTP_CONFIG_KEY_TOKEN_DIGITS) ||
        !flipper_format_delete_key(context->config_file, TOTP_CONFIG_KEY_TOKEN_DURATION) ||
        !flipper_format_delete_key(context->config_file, TOTP_CONFIG_KEY_TOKEN_AUTOMATION_FEATURES)) {
        return false;
    }

    context->total_count--;
    if (context->current_index >= context->total_count) {
        context->current_index = context->total_count - 1;
    }

    return true;
}

bool totp_token_info_iterator_save_current_token_info_changes(TokenInfoIteratorContext* context) {
    if (context->current_index < context->total_count) {
        if (!seek_to_token(context->current_index, context)) {
            return false;
        }
    } else {
        if (!flipper_format_seek_to_end(context->config_file)) {
            return false;
        }
    }

    TokenInfo* token_info = context->current_token;

    if(!flipper_format_insert_or_update_string(context->config_file, TOTP_CONFIG_KEY_TOKEN_NAME, token_info->name_n)) {
        return false;
    }

    if(!flipper_format_insert_or_update_hex(
            context->config_file, TOTP_CONFIG_KEY_TOKEN_SECRET, token_info->token, token_info->token_length)) {
        return false;
    }

    uint32_t tmp_uint32 = token_info->algo;
    if(!flipper_format_insert_or_update_uint32(
            context->config_file, TOTP_CONFIG_KEY_TOKEN_ALGO, &tmp_uint32, 1)) {
        return false;
    }

    tmp_uint32 = token_info->digits;
    if(!flipper_format_insert_or_update_uint32(context->config_file, TOTP_CONFIG_KEY_TOKEN_DIGITS, &tmp_uint32, 1)) {
        return false;
    }

    tmp_uint32 = token_info->duration;
    if(!flipper_format_insert_or_update_uint32(context->config_file, TOTP_CONFIG_KEY_TOKEN_DURATION, &tmp_uint32, 1)) {
        return false;
    }

    tmp_uint32 = token_info->automation_features;
    if(!flipper_format_insert_or_update_uint32(
            context->config_file, TOTP_CONFIG_KEY_TOKEN_AUTOMATION_FEATURES, &tmp_uint32, 1)) {
        return false;
    }

    if (context->current_index >= context->total_count) {
        context->total_count++;
    }

    return true;
}

bool totp_token_info_iterator_load_current_token_info(TokenInfoIteratorContext* context) {
    furi_check(context != NULL);
    if (!seek_to_token(context->current_index, context)) {
        return false;
    }

    if (!flipper_format_read_string(context->config_file, TOTP_CONFIG_KEY_TOKEN_NAME, context->current_token->name_n)) {
        return false;
    }

    uint32_t secret_bytes_count;
    if(!flipper_format_get_value_count(
            context->config_file, TOTP_CONFIG_KEY_TOKEN_SECRET, &secret_bytes_count)) {
        secret_bytes_count = 0;
    }
    TokenInfo* tokenInfo = context->current_token;
    bool token_update_needed = false;
    if(secret_bytes_count == 1) { // Plain secret key
        FuriString* temp_str = furi_string_alloc();
        if(flipper_format_read_string(context->config_file, TOTP_CONFIG_KEY_TOKEN_SECRET, temp_str)) {
            if(token_info_set_secret(
                    tokenInfo,
                    furi_string_get_cstr(temp_str),
                    furi_string_size(temp_str),
                    PLAIN_TOKEN_ENCODING_BASE32,
                    context->iv)) {
                FURI_LOG_W(LOGGING_TAG, "Token \"%s\" has plain secret", furi_string_get_cstr(tokenInfo->name_n));
                token_update_needed = true;
            } else {
                tokenInfo->token = NULL;
                tokenInfo->token_length = 0;
                FURI_LOG_W(LOGGING_TAG, "Token \"%s\" has invalid secret", furi_string_get_cstr(tokenInfo->name_n));
            }
        } else {
            tokenInfo->token = NULL;
            tokenInfo->token_length = 0;
        }

        furi_string_free(temp_str);
    } else { // encrypted
        tokenInfo->token_length = secret_bytes_count;
        if(secret_bytes_count > 0) {
            tokenInfo->token = malloc(tokenInfo->token_length);
            furi_check(tokenInfo->token != NULL);
            if(!flipper_format_read_hex(
                    context->config_file,
                    TOTP_CONFIG_KEY_TOKEN_SECRET,
                    tokenInfo->token,
                    tokenInfo->token_length)) {
                free(tokenInfo->token);
                tokenInfo->token = NULL;
                tokenInfo->token_length = 0;
            }
        } else {
            tokenInfo->token = NULL;
        }
    }

    uint32_t temp_data32;
    if(flipper_format_read_uint32(context->config_file, TOTP_CONFIG_KEY_TOKEN_ALGO, &temp_data32, 1) &&
        temp_data32 <= STEAM) {
        tokenInfo->algo = (TokenHashAlgo)temp_data32;
    } else {
        tokenInfo->algo = SHA1;
    }

    if(!flipper_format_read_uint32(
            context->config_file, TOTP_CONFIG_KEY_TOKEN_DIGITS, &temp_data32, 1) ||
        !token_info_set_digits_from_int(tokenInfo, temp_data32)) {
        tokenInfo->digits = TOTP_6_DIGITS;
    }

    if(!flipper_format_read_uint32(
            context->config_file, TOTP_CONFIG_KEY_TOKEN_DURATION, &temp_data32, 1) ||
        !token_info_set_duration_from_int(tokenInfo, temp_data32)) {
        tokenInfo->duration = TOTP_TOKEN_DURATION_DEFAULT;
    }

    if(flipper_format_read_uint32(
            context->config_file, TOTP_CONFIG_KEY_TOKEN_AUTOMATION_FEATURES, &temp_data32, 1)) {
        tokenInfo->automation_features = temp_data32;
    } else {
        tokenInfo->automation_features = TOKEN_AUTOMATION_FEATURE_NONE;
    }

    if (token_update_needed) {
        if (!totp_token_info_iterator_save_current_token_info_changes(context)) {
            return false;
        }
    }

    return true;
}