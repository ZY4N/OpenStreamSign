#pragma once

#include <variant>
#include "color_suppliers/random_color_supplier.hpp"
#include "color_suppliers/sequence_color_supplier.hpp"

using namespace ztu::uix;

template<class color_suppliers_t>
using color_supplier = std::variant<
	typename color_suppliers_t::random,
	typename color_suppliers_t::sequence
>;

template<u32 SequenceLength>
struct color_suppliers {
	using random = color_suppliers_detail::random_color_supplier;
	using sequence = color_suppliers_detail::sequence_color_supplier<SequenceLength>;
};
