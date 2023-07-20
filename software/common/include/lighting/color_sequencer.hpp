#pragma once


#include <util/uix.hpp>
#include <util/variant_visit.hpp>
#include <array>

#include "color_supplier.hpp"
#include "color_mixing.hpp"

template<usize TicksPerColor, class color_supplier_t>
struct color_sequencer {

	void init() {
		ztu::visit([](auto &theSupplier) {
			theSupplier.init();
		}, supplier);
	}

	color operator()(const u32 t) {
		const auto t_color = t / TicksPerColor;
		const auto t_mix = t - t_color * TicksPerColor;

		color a, b;

		ztu::visit([&](auto &supply) {
			a = supply(t_color);
			b = supply(t_color + 1);
		}, supplier);

		return color_mixing::mix(
			mixType, a, b,
			static_cast<float>(t_mix) / static_cast<float>(TicksPerColor)
		);
	}

	constexpr bool operator==(const color_sequencer<TicksPerColor, color_supplier_t>&) const = default;

	color_supplier_t supplier;
	color_mixing::type mixType;
};
