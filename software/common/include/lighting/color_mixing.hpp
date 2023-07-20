#pragma once

#include "color.hpp"

namespace color_mixing {
	enum class type : uint8_t {
		LINEAR_INTERPOLATION	= 0,	// __/"""/'"'\'''\__
		FADE_IN_OUT				= 1,	// __/"""\___/'''\__
		PWM						= 2,	// __|""|____|''|___
		RAMP					= 3,	// ____**""|_**''|__
		NO_MIXING				= 4		// __|"""""|'''''|__
	};

	inline constexpr color mix(type mixingType, const color &a, const color &b, float t);

	template<type MixingType>
	inline constexpr color mix(const color &a, const color &b, float t);
};

#define INCLUDE_COLOR_MIXING_IMPLEMENTATION
#include <lighting/color_mixing.ipp>
#undef INCLUDE_COLOR_MIXING_IMPLEMENTATION
