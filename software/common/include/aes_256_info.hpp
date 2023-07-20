#pragma once

#include <util/uix.hpp>

namespace aes_256_info {

	using ztu::usize;

	inline constexpr usize blockSize = 16;
	inline constexpr usize ivSize = blockSize;

	constexpr usize cipherLength(const usize textLength) {
		return (textLength / blockSize + 1) * blockSize;
	}
}
