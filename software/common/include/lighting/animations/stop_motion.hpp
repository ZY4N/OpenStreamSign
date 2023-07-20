#pragma once

#include <util/uix.hpp>
#include <array>
#include <span>
#include <cassert>
#include <algorithm>


namespace animation_detail {

	template<u32 TicksPerColor, u32 NumPixels, u32 MaxFrames>
	struct stop_motion {

		static constexpr auto numPixels = NumPixels;
		static constexpr auto maxFrames = MaxFrames;

		constexpr stop_motion() = default;

		constexpr stop_motion(std::span<const color> newFrames)
			: numFrames{ static_cast<u16>(newFrames.size()) }
		{
			assert(numFrames == newFrames.size());
			assert(numFrames % NumPixels == 0);
			assert(numFrames / NumPixels <= MaxFrames);
			std::copy(newFrames.begin(), newFrames.end(), m_frames.begin());
		}

		void init() {}

		void operator()(std::span<color> dst, const u32 t) {
			assert(numFrames > 0 && numFrames <= MaxFrames);

			const auto frameIndex = (t % TicksPerColor) / numFrames;
			const auto frame = &m_frames[frameIndex * NumPixels];

			for (isize i = 0, n = dst.size(); i < n; i++) {
				dst[i] = frame[i % NumPixels];
			}
		}

		std::span<const color> frames() const {
			return { m_frames.begin(), m_frames.begin() + numFrames };
		}

		std::span<color> frames() {
			return { m_frames.begin(), m_frames.begin() + numFrames };
		}

		constexpr bool operator==(const stop_motion<TicksPerColor, NumPixels, MaxFrames> &other) const {
			return (
				other.numFrames == numFrames and
				std::ranges::equal(
					std::span{ m_frames.begin(), numFrames },
					std::span{ other.m_frames.begin(), numFrames }
				)
			);
		}
		
		std::array<color, NumPixels * MaxFrames> m_frames;
		u16 numFrames{ 0 };
	};

}
