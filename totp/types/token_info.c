#include <furi/furi.h>
#include <furi_hal.h>
#include "token_info.h"
#include "stdlib.h"
#include "common.h"
#include "../services/base32/base32.h"
#include "../services/crypto/crypto.h"
#include "../services/crypto/memset_s.h"

TokenInfo* token_info_alloc() {
    TokenInfo* tokenInfo = malloc(sizeof(TokenInfo));
    tokenInfo->algo = SHA1;
    tokenInfo->digits = TOTP_6_DIGITS;
    return tokenInfo;
}

void token_info_free(TokenInfo* token_info) {
    if(token_info == NULL) return;
    free(token_info->name);
    free(token_info->token);
    free(token_info);
}

bool token_info_set_secret(
    TokenInfo* token_info,
    const char* base32_token_secret,
    size_t token_secret_length,
    uint8_t* iv) {
    uint8_t* plain_secret = malloc(token_secret_length);
    int plain_secret_length =
        base32_decode((uint8_t*)base32_token_secret, plain_secret, token_secret_length);
    bool result;
    if(plain_secret_length >= 0) {
        token_info->token =
            totp_crypto_encrypt(plain_secret, plain_secret_length, iv, &token_info->token_length);
        result = true;
    } else {
        result = false;
    }

    memset_s(plain_secret, sizeof(plain_secret), 0, token_secret_length);
    free(plain_secret);
    return result;
}

uint8_t token_info_get_digits_count(TokenInfo* token_info) {
    switch(token_info->digits) {
    case TOTP_6_DIGITS:
        return 6;
    case TOTP_8_DIGITS:
        return 8;
    }

    return 6;
}
