#pragma once

#include <variant>
#include <algorithm>

#include "animations/uniform_color.hpp"
#include "animations/moving_colors.hpp"
#include "animations/moving_pixel.hpp"
#include "animations/stop_motion.hpp"

using namespace ztu::uix;

template<class animations_t>
using animation = std::variant<
	typename animations_t::uniform_color,
	typename animations_t::moving_colors,
	typename animations_t::moving_pixel,
	typename animations_t::stop_motion
>;

template<class animations_t>
struct variable_speed_animation {

	using animation_t = animation<animations_t>;

	constexpr variable_speed_animation() = default;

	variable_speed_animation(const animation_t &newAnimator)
		: animator{ newAnimator } {}

	variable_speed_animation(const animation_t &newAnimator, double secondsPerStage)
		: animator{ newAnimator }
	{
		const auto accurate_speed = std::round(
			static_cast<double>(animations_t::ticksPerColor) / (secondsPerStage * 30)
		);

		speed = static_cast<i8>(
			std::clamp(
				accurate_speed,
				static_cast<double>(I8_MIN),
				static_cast<double>(I8_MAX)
			)
		);
	}

	double duration() const {
		return static_cast<double>(animations_t::ticksPerColor) / (30.0 * static_cast<double>(speed));
	}

	constexpr bool operator==(const variable_speed_animation<animations_t>&) const = default;

	animation_t animator;
	i8 speed{ 1 };
};

template<u32 TicksPerColor, u32 NumPixels, u32 NumFrames, class color_sequencer_t, class temporal_scaler_t>
struct animations {
	static constexpr auto ticksPerColor = TicksPerColor;
	static constexpr auto numPixels = NumPixels;
	static constexpr auto numFrames = NumFrames;
	using uniform_color = animation_detail::uniform_color<color_sequencer_t>;
	using moving_colors = animation_detail::moving_colors<color_sequencer_t>;
	using moving_pixel = animation_detail::moving_pixel<color_sequencer_t, temporal_scaler_t>;
	using stop_motion = animation_detail::stop_motion<TicksPerColor, NumPixels, NumFrames>;
};
