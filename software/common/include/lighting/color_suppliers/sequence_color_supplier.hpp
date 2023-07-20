#pragma once

#include "../color.hpp"
#include <util/uix.hpp>
#include <array>
#include <span>
#include <utility>
#include <cassert>

namespace color_suppliers_detail {

	template<usize NumColors>
	struct sequence_color_supplier {

		template<typename... Ts>
			requires (
				sizeof...(Ts) <= NumColors &&
				(std::same_as<Ts, color> && ...)
			)
		constexpr sequence_color_supplier(const Ts&... newColors)
			: m_colors{ newColors... }, numColors{ sizeof...(Ts) } {}

		constexpr sequence_color_supplier(std::span<const color> newColors) {
			assert(newColors.size() <= NumColors);
			std::copy(newColors.begin(), newColors.end(), m_colors.begin());
			numColors = newColors.size();
		}

		void init() {}

		color operator()(u32 index) {
			return m_colors[index % numColors];
		}

		std::span<const color> colors() const {
			return { m_colors.begin(), m_colors.begin() + numColors };
		}

		std::span<color> colors() {
			return { m_colors.begin(), m_colors.begin() + numColors };
		}

		usize size() const {
			return numColors;
		}

		constexpr bool operator==(const sequence_color_supplier<NumColors> &other) const {
			return std::ranges::equal(colors(), other.colors());
		}

		std::array<color, NumColors> m_colors;
		u32 numColors;
	};
	
}
