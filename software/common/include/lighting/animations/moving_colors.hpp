#pragma once

#include <span>
#include <cmath>

#include "../color_sequencer.hpp"

namespace animation_detail {

	template<class color_sequencer_t>
	struct moving_colors {

		constexpr moving_colors() = default;

		constexpr moving_colors(
			const color_sequencer_t &newSequencer,
			u32 newPerPixelOffset = 1,
			u32 newPixelOffset = 0
		) :
			sequencer{ newSequencer },
			perPixelOffset{ newPerPixelOffset },
			pixelOffset{ newPixelOffset } {}

		void init() {
			sequencer.init();
		}

		void operator()(std::span<color> dst, const u32 t) {
			auto t_local = static_cast<i32>(pixelOffset * perPixelOffset);
			for (isize i = 0, n = dst.size(); i < n; i++) {
				const auto t_pixel = static_cast<int>(i * perPixelOffset);
				dst[i] = sequencer(t - std::abs(t_pixel - t_local));
			}
		}

		constexpr bool operator==(const moving_colors<color_sequencer_t>&) const = default;

		color_sequencer_t sequencer;
		u32 perPixelOffset, pixelOffset;
	};
}
