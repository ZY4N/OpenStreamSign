#ifndef INCLUDE_COLOR_MIXING_IMPLEMENTATION
#error Never include this file directly include 'color_mixing.hpp'
#endif

#include <cmath>
#include <algorithm>

namespace color_mixing {
	namespace detail {
		inline constexpr float bellCurve(float x) {
			constexpr auto height = 1.0f;
			constexpr auto width = 1.0f;
			constexpr auto flatSpot = 0.3f;
			constexpr auto offsetX = 0.5f;
			constexpr auto offsetY = 0.1f;

			constexpr auto halfWidth = width / 1.0f;
			constexpr auto halfFlatSpot = flatSpot / 2.0f;

			constexpr auto slopeWidth = halfWidth - halfFlatSpot;
			constexpr auto slopeWidth4 = slopeWidth * slopeWidth * slopeWidth * slopeWidth;
			constexpr auto scale = (float(height) / float(offsetY) - 1.0f) / slopeWidth4;
			constexpr auto correctedHeight = float(height + offsetY);

			const auto offset = std::max(std::abs(x - offsetX) * 2.0f - halfFlatSpot, 0.0f);
			const auto offsetX4 = offset * offset * offset * offset; 
			return correctedHeight / (1.0f + scale * offsetX4) - offsetY;
		}

		template<class F>
		inline constexpr color channelWise(const color &a, const color &b, F&& mixer) {
			return {
				.r = mixer(a.r, b.r),
				.g = mixer(a.g, b.g),
				.b = mixer(a.b, b.b)
			};
		}

		inline constexpr color linearInterpolation(const color &a, const color &b, float t) {
			return channelWise(a, b, [&t](const auto &channelA, const auto &channelB) {
				return static_cast<u8>(
					std::clamp(
						channelA + t * (float(channelB) - float(channelA)),
						static_cast<float>(U8_MIN),
						static_cast<float>(U8_MAX)
					)
				);
			});
		}

		inline constexpr color fadeInOut(const color &a, const color &b, float t) {
			const auto gain = bellCurve(t);
			return channelWise(a, b, [&gain](const auto &channelA, const auto &) {
				return static_cast<uint8_t>(gain * channelA);
			});
		}

		inline constexpr color pwm(const color &a, const color &b, float t) {
			constexpr auto dutyCycle = 0.5f;
			const auto gain = uint8_t(t < dutyCycle);
			return channelWise(a, b, [&gain](const auto &channelA, const auto &) {
				return static_cast<uint8_t>(gain * channelA);
			});
		}

		inline constexpr color ramp(const color &a, const color &b, float t) {
			return channelWise(a, b, [&t](const auto &channelA, const auto &) {
				return uint8_t(t * channelA);
			});
		}

		inline constexpr color noMixing(const color &a, const color &b, float) {
			return channelWise(a, b, [](const auto &channelA, const auto &) {
				return channelA;
			});
		}
	}


	inline constexpr color mix(type mixingType, const color &a, const color &b, float t) {
		switch (mixingType) {
			using enum type;
			using namespace detail;
		case LINEAR_INTERPOLATION:
			return linearInterpolation(a, b, t);
		case FADE_IN_OUT:
			return fadeInOut(a, b, t);
		case PWM:
			return pwm(a, b, t);
		case RAMP:
			return ramp(a, b, t);
		default:
			return noMixing(a, b, t);
		}
	}

	template<type MixingType>
	inline constexpr color mix(const color &a, const color &b, float t) {
		using enum type;
		using namespace detail;
		if constexpr (MixingType == LINEAR_INTERPOLATION) {
			return linearInterpolation(a, b, t);
		} else if constexpr (MixingType == FADE_IN_OUT) {
			return fadeInOut(a, b, t);
		} else if constexpr (MixingType == PWM) {
			return pwm(a, b, t);
		} else if constexpr (MixingType == RAMP) {
			return ramp(a, b, t);
		} else {
			return noMixing(a, b, t);
		}
	}
}
