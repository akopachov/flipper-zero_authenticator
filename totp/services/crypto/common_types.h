#pragma once

#include <stdint.h>

typedef uint8_t CryptoSeedIVResult;

enum CryptoSeedIVResults {

    /**
     * @brief IV seeding operation failed
     */
    CryptoSeedIVResultFailed = 0b000,

    /**
     * @brief IV seeding operation succeeded
     */
    CryptoSeedIVResultFlagSuccess = 0b001,

    /**
     * @brief As a part of IV seeding operation new crypto verify data has been generated
     */
    CryptoSeedIVResultFlagNewCryptoVerifyData = 0b010
};