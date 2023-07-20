#pragma once

#include "../color_sequencer.hpp"

namespace animation_detail {
	
	template<class color_sequencer_t>
	struct uniform_color {

		constexpr uniform_color() = default;

		constexpr uniform_color(const color_sequencer_t &newSequencer)
			: sequencer{ newSequencer } {}

		void init() {
			sequencer.init();
		}

		void operator()(std::span<color> dst, const u32 t) {
			std::fill(dst.begin(), dst.end(), sequencer(t));
		}

		constexpr bool operator==(const uniform_color<color_sequencer_t> &other) const = default;
		
		color_sequencer_t sequencer;
	};

}
