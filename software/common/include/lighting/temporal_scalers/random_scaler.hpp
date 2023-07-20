#pragma once

#include <util/uix.hpp>
#include <util/hash_u32.hpp>
#include <fill_random.hpp>

namespace temporal_scalers_detail {

	// '''''*****`````,,
	template<u32 TicksPerColor>
	class random_scaler {
	public:
		constexpr random_scaler() = default;

		void init() {
			const auto begin = reinterpret_cast<u8*>(&salt); 
			fill_random({ begin, begin + sizeof(salt) });
		}

		float operator()(u32 t) {
			constexpr auto scale = 1.0f / static_cast<float>(U32_MAX);
			const auto rnd = ztu::hash_u32(salt + t / TicksPerColor);
			return scale * static_cast<float>(rnd);
		}

		constexpr bool operator==(const random_scaler<TicksPerColor> &) const {
			return true;
		}

	private:
		u32 salt{ 0 };
	};
	
}
