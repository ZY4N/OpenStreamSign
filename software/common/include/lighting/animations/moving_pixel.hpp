#pragma once

#include "../color_sequencer.hpp"
#include "../temporal_scaler.hpp"
#include "../color_mixing.hpp"

#include <util/variant_visit.hpp>

namespace animation_detail {

	template<class color_sequencer_t, class temporal_scaler_t>
	struct moving_pixel {

		constexpr moving_pixel() = default;

		constexpr moving_pixel(
			const color_sequencer_t &newSequencer,
			const temporal_scaler_t &newScaler,
			double newColorSpeed = 1.0f,
			u8 newWidth = 1
		) :
			sequencer{ newSequencer },
			scaler{ newScaler },
			colorSpeed{ static_cast<float>(newColorSpeed) },
			width{ newWidth } {}
		

		void init() {
			sequencer.init();
			ztu::visit([](auto &theScaler) {
				theScaler.init();
			}, scaler);
		}

		void operator()(std::span<color> dst, const u32 t) {
		
			auto offset = 0.0f;
			ztu::visit([&](auto &scale) {
				offset = (static_cast<float>(dst.size() - width) - std::numeric_limits<float>::epsilon()) * scale(static_cast<u32>(t));
			}, scaler);
			
			const auto minPixel = static_cast<u32>(std::floor(offset));
			const auto maxPixel = minPixel + width;
			float a = offset - minPixel;

			const auto c = sequencer(static_cast<u32>(std::round(colorSpeed * t)));

			constexpr auto colorLerp = &color_mixing::mix<color_mixing::type::LINEAR_INTERPOLATION>;

			auto i = 0U;
			while (i < minPixel)	dst[i++] = colors::black;
									dst[i++] = colorLerp(c, colors::black, a);
			while (i < maxPixel)	dst[i++] = c;
									dst[i++] = colorLerp(colors::black, c, a);
			while (i < dst.size())	dst[i++] = colors::black;

		}


		constexpr bool operator==(const moving_pixel<color_sequencer_t, temporal_scaler_t>&) const = default;

		color_sequencer_t sequencer;
		temporal_scaler_t scaler;
		float colorSpeed; static_assert(sizeof(colorSpeed == 4));
		u8 width;
	};

}
