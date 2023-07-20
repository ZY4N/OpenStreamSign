#pragma once

#include <span>
#include <random>
#include <algorithm>


inline void mt19937_random_fill(std::span<uint8_t> dst) {
	static std::random_device rd;
	static std::mt19937 rng(rd());

	using limits = std::numeric_limits<uint8_t>;

	std::uniform_int_distribution<uint8_t> dist(limits::min(), limits::max());

	std::generate(dst.begin(), dst.end(), [&]() {
		return dist(rng);
	});
}
