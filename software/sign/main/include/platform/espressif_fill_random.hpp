#pragma once

#include "esp_random.h"
#include <span>

inline void espressif_fill_random(std::span<uint8_t> dst) {
	esp_fill_random(dst.data(), dst.size());
}
