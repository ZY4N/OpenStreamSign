#pragma once

#include "uix.hpp"

namespace ztu {
	// https://stackoverflow.com/a/12996028
	__attribute__((always_inline)) inline u32 hash_u32(u32 x) {
		x = ((x >> 16) ^ x) * 0x45d9f3b;
		x = ((x >> 16) ^ x) * 0x45d9f3b;
		x = (x >> 16) ^ x;
		return x;
	};
}
