#include "crypto_v2.h"
#include <furi_hal_crypto.h>
#include <furi_hal_random.h>
#include <furi_hal_version.h>
#include "../../types/common.h"
#include "memset_s.h"
#include "crypto_constants_v2.h"

#define CRYPTO_ALIGNMENT_FACTOR (16)

static const uint8_t* get_device_uid() {
    return (const uint8_t*)UID64_BASE; //-V566
}

static uint8_t get_device_uid_length() { 
    return furi_hal_version_uid_size();
}

static const uint8_t* get_crypto_verify_key() {
    return get_device_uid();
}

static uint8_t get_crypto_verify_key_length() {
    return get_device_uid_length();
}

bool totp_crypto_check_key_slot(uint8_t key_slot) {
    uint8_t iv[CRYPTO_IV_LENGTH];
    furi_hal_random_fill_buf(&iv[0], CRYPTO_IV_LENGTH);
    return furi_hal_crypto_store_load_key(key_slot, iv) && furi_hal_crypto_store_unload_key(key_slot);
}

uint8_t* totp_crypto_encrypt(
    const uint8_t* plain_data,
    const size_t plain_data_length,
    const uint8_t* iv,
    uint8_t key_slot,
    size_t* encrypted_data_length) {
    uint8_t* encrypted_data;
    size_t remain = plain_data_length % CRYPTO_ALIGNMENT_FACTOR;
    if(remain) {
        size_t plain_data_aligned_length = plain_data_length - remain + CRYPTO_ALIGNMENT_FACTOR;
        uint8_t* plain_data_aligned = malloc(plain_data_aligned_length);
        furi_check(plain_data_aligned != NULL);
        memset(plain_data_aligned, 0, plain_data_aligned_length);
        memcpy(plain_data_aligned, plain_data, plain_data_length);

        encrypted_data = malloc(plain_data_aligned_length);
        furi_check(encrypted_data != NULL);
        *encrypted_data_length = plain_data_aligned_length;

        furi_check(
            furi_hal_crypto_store_load_key(key_slot, iv) &&
            furi_hal_crypto_encrypt(plain_data_aligned, encrypted_data, plain_data_aligned_length) &&
            furi_hal_crypto_store_unload_key(key_slot), "Encryption failed");

        memset_s(plain_data_aligned, plain_data_aligned_length, 0, plain_data_aligned_length);
        free(plain_data_aligned);
    } else {
        encrypted_data = malloc(plain_data_length);
        furi_check(encrypted_data != NULL);
        *encrypted_data_length = plain_data_length;

        furi_check(
            furi_hal_crypto_store_load_key(key_slot, iv) &&
            furi_hal_crypto_encrypt(plain_data, encrypted_data, plain_data_length) &&
            furi_hal_crypto_store_unload_key(key_slot), "Encryption failed");
    }

    return encrypted_data;
}

uint8_t* totp_crypto_decrypt(
    const uint8_t* encrypted_data,
    const size_t encrypted_data_length,
    const uint8_t* iv,
    uint8_t key_slot,
    size_t* decrypted_data_length) {
    *decrypted_data_length = encrypted_data_length;
    uint8_t* decrypted_data = malloc(*decrypted_data_length);
    furi_check(decrypted_data != NULL);
    furi_check(
        furi_hal_crypto_store_load_key(key_slot, iv) &&
        furi_hal_crypto_decrypt(encrypted_data, decrypted_data, encrypted_data_length) &&
        furi_hal_crypto_store_unload_key(key_slot), "Decryption failed");
    return decrypted_data;
}

CryptoSeedIVResult
    totp_crypto_seed_iv(PluginState* plugin_state, const uint8_t* pin, uint8_t pin_length) {
    CryptoSeedIVResult result;
    if(plugin_state->crypto_verify_data == NULL) {
        FURI_LOG_I(LOGGING_TAG, "Generating new IV");
        furi_hal_random_fill_buf(&plugin_state->base_iv[0], CRYPTO_IV_LENGTH);
    }

    memcpy(&plugin_state->iv[0], &plugin_state->base_iv[0], CRYPTO_IV_LENGTH);

    const uint8_t* device_uid = get_device_uid();
    uint8_t device_uid_length = get_device_uid_length();

    for(uint8_t i = 0; i < CRYPTO_IV_LENGTH && i < device_uid_length; i++) {
        plugin_state->iv[i] = plugin_state->iv[i] ^ device_uid[i];
    }

    if(pin != NULL && pin_length > 0) {
        for(uint8_t i = 0; i < pin_length && i < CRYPTO_IV_LENGTH; i++) {
            uint8_t factor;
            if (i < device_uid_length) {
                factor = device_uid[device_uid_length - i - 1];
            } else {
                factor = i + 1;
            }

            plugin_state->iv[i] = plugin_state->iv[i] ^ (uint8_t)(pin[i] * factor);
        }
    }

    result = CryptoSeedIVResultFlagSuccess;
    if(plugin_state->crypto_verify_data == NULL) {
        const uint8_t* crypto_vkey = get_crypto_verify_key();
        uint8_t crypto_vkey_length = get_crypto_verify_key_length();
        FURI_LOG_I(LOGGING_TAG, "Generating crypto verify data");
        plugin_state->crypto_verify_data = malloc(crypto_vkey_length);
        furi_check(plugin_state->crypto_verify_data != NULL);
        plugin_state->crypto_verify_data_length = crypto_vkey_length;

        plugin_state->crypto_verify_data = totp_crypto_encrypt(
            crypto_vkey,
            crypto_vkey_length,
            &plugin_state->iv[0],
            plugin_state->crypto_key_slot,
            &plugin_state->crypto_verify_data_length);

        plugin_state->pin_set = pin != NULL && pin_length > 0;

        result |= CryptoSeedIVResultFlagNewCryptoVerifyData;
    }

    return result;
}

bool totp_crypto_verify_key(const PluginState* plugin_state) {
    size_t decrypted_key_length;
    uint8_t* decrypted_key = totp_crypto_decrypt(
        plugin_state->crypto_verify_data,
        plugin_state->crypto_verify_data_length,
        &plugin_state->iv[0],
        plugin_state->crypto_key_slot,
        &decrypted_key_length);

    const uint8_t* crypto_vkey = get_crypto_verify_key();
    uint8_t crypto_vkey_length = get_crypto_verify_key_length();
    bool key_valid = true;
    for(uint8_t i = 0; i < crypto_vkey_length && key_valid; i++) {
        if(decrypted_key[i] != crypto_vkey[i]) key_valid = false;
    }

    free(decrypted_key);

    return key_valid;
}