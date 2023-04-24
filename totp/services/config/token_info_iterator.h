#pragma once

#include "../../types/token_info.h"
#include <flipper_format/flipper_format.h>
#include "constants.h"

typedef struct {
    size_t total_count;
    size_t current_index;
    size_t last_seek_offset;
    size_t last_seek_index;
    TokenInfo* current_token;
    FlipperFormat* config_file;
    uint8_t* iv;
} TokenInfoIteratorContext;

TokenInfoIteratorContext* totp_token_info_iterator_alloc(FlipperFormat* config_file, uint8_t* iv);

bool totp_token_info_iterator_load_current_token_info(TokenInfoIteratorContext* context);

bool totp_token_info_iterator_move_current_token_info(
    TokenInfoIteratorContext* context,
    size_t new_index);

bool totp_token_info_iterator_save_current_token_info_changes(TokenInfoIteratorContext* context);

bool totp_token_info_iterator_remove_current_token_info(TokenInfoIteratorContext* context);

void totp_token_info_iterator_free(TokenInfoIteratorContext* context);