#pragma once

#include <stdlib.h>
#include <stdint.h>

uint8_t* base64_decode(const uint8_t* src, size_t len, size_t* out_len, size_t* out_size);