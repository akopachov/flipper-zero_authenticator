#pragma once

#include "font_info.h"
#include <stdbool.h>
#include <stddef.h>
#include "../../types/automation_kb_layout.h"

#define TOTP_DEFAULT_KB_LAYOUT "en-US"

/**
 * @brief Loads \c kb_layout layout into \c buffer
 * @param kb_layout keyboard layout name
 * @param buffer buffer to load keyboard layout to
 * @return \c true if keyboard layout loaded successfully; \c false otherwise
 */
bool totp_kb_layout_provider_get_layout(char* kb_layout, uint16_t* buffer);

/**
 * @brief Gets keyboard layout index by keyboard layout name
 * @param kb_layout keyboard layout name
 * @return keyboard layout index
 */
uint8_t totp_kb_layout_provider_get_layout_index(char* kb_layout);

/**
 * @brief Gets total keyboard layouts count
 * @return total keyboard layouts count
 */
uint8_t totp_kb_layout_provider_get_layouts_count();

/**
 * @brief Loads \c kb_layout layout into \c buffer
 * @param kb_layout keyboard layout name
 * @param buffer buffer to load keyboard layout to
 * @return \c true if keyboard layout loaded successfully; \c false otherwise
 */
bool totp_kb_layout_provider_get_layout_name_by_index(uint8_t layout_index, char* buffer);
