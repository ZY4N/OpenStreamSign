#pragma once

#include <concepts/fill_random_concept.hpp>
#include <platform/espressif_fill_random.hpp>

static_assert(fill_random_concept<decltype(espressif_fill_random)>);

inline auto &fill_random = espressif_fill_random;
