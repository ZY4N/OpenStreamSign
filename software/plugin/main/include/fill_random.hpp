#pragma once

#include <concepts/fill_random_concept.hpp>
#include <platform/mt19937_random_fill.hpp>

static_assert(fill_random_concept<decltype(mt19937_random_fill)>);

inline auto &fill_random = mt19937_random_fill;
