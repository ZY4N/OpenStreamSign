#pragma once

#include <util/uix.hpp>
#include <cmath>

namespace temporal_scalers_detail {

	// __****''`''****__
	template<u32 TicksPerColor>
	class ping_pong_scaler {
	public:
		constexpr ping_pong_scaler() = default;

		void init() {}

		float operator()(u32 t) {
			const auto a = static_cast<float>(t % TicksPerColor) / static_cast<float>(TicksPerColor);
			return 1.0f - std::abs(2.0f * a - 1.0f);
		}

		constexpr bool operator==(const ping_pong_scaler<TicksPerColor>&) const = default;
	};
	
}
