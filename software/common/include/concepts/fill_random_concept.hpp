#pragma once

#include <util/uix.hpp>
#include <span>

template<typename F>
concept fill_random_concept = requires(F f, std::span<ztu::u8> buffer) {
	{ f(buffer) } -> std::same_as<void>;
};
