#pragma once

#include <flipper_format/flipper_format.h>
#include "token_info_iterator.h"

typedef struct {
    FlipperFormat* config_file;

    Storage* storage;

    /**
     * @brief Token list iterator context 
     */
    TokenInfoIteratorContext* token_info_iterator_context;
} ConfigFileContext;