#pragma once

#include <util/uix.hpp>
#include <cmath>
#include <numbers>

namespace temporal_scalers_detail {

	// -**'`'**--**.,.**-
	template<u32 TicksPerColor>
	class sinus_scaler {
	public:
		constexpr sinus_scaler() = default;

		void init() {}

		float operator()(u32 t) {
			constexpr auto TWO_PI = 2.0f * std::numbers::pi_v<float>; 
			const auto a = static_cast<float>(t) / static_cast<float>(TicksPerColor);
			return 0.5f * std::sin(TWO_PI * a) + 0.5f;
		}

		constexpr bool operator==(const sinus_scaler<TicksPerColor> &) const {
			return true;
		}
	};
}
