#pragma once

#include <util/uix.hpp>
#include <util/hash_u32.hpp>
#include <fill_random.hpp>
#include <cmath>

namespace temporal_scalers_detail {

	// ```````*******-----
	template<u32 TicksPerColor>
	class smooth_random_scaler {
	public:
		constexpr smooth_random_scaler() = default;

		void init() {
			const auto begin = reinterpret_cast<u8*>(&salt); 
			fill_random({ begin, begin + sizeof(salt) });
		}

		float operator()(u32 t) {
			constexpr auto scale = 1.0f / static_cast<float>(U32_MAX);

			const auto t0 = t / TicksPerColor;
			const auto from = scale * ztu::hash_u32(salt + t0);
			const auto to = scale * ztu::hash_u32(salt + t0 + 1);

			const auto a = static_cast<float>(t - t0 * TicksPerColor) / static_cast<float>(TicksPerColor);

			const auto smoothLerp = [](float x, float y, float a) {
				a -= 0.5f;
				return x + (y - x) * (0.6f * (a / (0.1f + std::abs(a))) + 0.5);
			};

			return smoothLerp(from, to, a);
		}

		constexpr bool operator==(const smooth_random_scaler<TicksPerColor> &) const {
			return true;
		}
		
	private:
		u32 salt{ 0 };
	};
	
}
