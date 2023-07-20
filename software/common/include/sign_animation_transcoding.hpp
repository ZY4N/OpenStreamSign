#pragma once

#include <domain_logic/sign_animation.hpp>
#include <util/for_each.hpp>

// This file is dedicated to C++'s enraging lack of reflection

namespace sign_animation_transcoding {
	namespace detail {
		template<typename T, typename It>
		inline void serializePrimitive(const T &value, It &dst) {
			static_assert(std::is_trivially_copyable_v<T>);
			using array_t = std::array<u8, sizeof(T)>;
			const auto &bytes = *reinterpret_cast<const array_t*>(&value);
			std::copy(bytes.begin(), bytes.end(), dst);
			dst += bytes.size();
		}

		template<typename S, typename T, size_t maxSize, typename It>
		inline bool serializePrimitiveArray(std::span<const T, maxSize> values, It &dst) {
			serializePrimitive(static_cast<S>(values.size()), dst);
			for (const auto& value : values) {
				serializePrimitive(value, dst);
			}
			return true;
		}

		template<typename It>
		inline void serializeSupplier(const sign_supplier &value, It &dst) {
			const auto index = value.index();
			*dst++ = static_cast<u8>(index);
			// index 0 is 'random' and does not contain relevant data
			if (index == 1) {
				const auto &sequence = std::get<sign_suppliers::sequence>(value);
				using seq_size_t = std::remove_cvref_t<decltype(sequence.numColors)>;
				serializePrimitiveArray<seq_size_t>(sequence.colors(), dst);
			}
		}

		template<typename It>
		inline void serializeScaler(const sign_scaler &value, It &dst) {
			*dst++ = static_cast<u8>(value.index());
			// node of the scalers contain relevant data
		}

		template<typename It>
		inline void serializeSequencer(const sign_sequencer &value, It &dst) {
			serializeSupplier(value.supplier, dst);
			serializePrimitive(value.mixType, dst);
		}
	}

	template<typename It>
	inline void serialize(const sign_animation &value, It &dst) {
		const auto &animation = value.animator;
		const auto index = animation.index();
		*dst++ = static_cast<u8>(index);
		switch (index) {
			case 0: {
				const auto &uniform_color = std::get<sign_animations::uniform_color>(animation);
				detail::serializeSequencer(uniform_color.sequencer, dst);
				break;
			}
			case 1: {
				const auto &moving_colors = std::get<sign_animations::moving_colors>(animation);
				detail::serializeSequencer(moving_colors.sequencer, dst);
				detail::serializePrimitive(moving_colors.perPixelOffset, dst);
				detail::serializePrimitive(moving_colors.pixelOffset, dst);
				break;
			}
			case 2: {
				const auto &moving_colors = std::get<sign_animations::moving_pixel>(animation);
				detail::serializeSequencer(moving_colors.sequencer, dst);
				detail::serializeScaler(moving_colors.scaler, dst);
				detail::serializePrimitive(moving_colors.colorSpeed, dst);
				detail::serializePrimitive(moving_colors.width, dst);
				break;
			}
			case 3: {
				const auto &stop_motion = std::get<sign_animations::stop_motion>(animation);
				using sm_size_t = std::remove_cvref_t<decltype(stop_motion.numFrames)>;
				detail::serializePrimitiveArray<sm_size_t>(stop_motion.frames(), dst);
				break;
			}
		}
		*dst++ = value.speed;
	}

	namespace detail {
		template<typename T, typename It>
		inline bool deserializePrimitive(T &value, It &src) {
			static_assert(std::is_trivially_copyable_v<T>);
			using array_t = std::array<u8, sizeof(T)>;
			auto &bytes = *reinterpret_cast<array_t*>(&value);
			std::copy(src, src + sizeof(T), bytes.begin());
			src += bytes.size();
			return true;
		}

		template<typename T, size_t maxSize, typename S, typename It>
		inline bool deserializePrimitiveArray(std::span<T, maxSize> dst, S &size, It &src) {
			deserializePrimitive(size, src);
			if (size > dst.size()) return false;
			for (S i = 0; i < size; i++) {
				deserializePrimitive(dst[i], src);
			}
			return true;
		}

		template<typename It>
		inline bool deserializeSupplier(sign_supplier &value, It &src) {
			const auto index = *src++;
			switch (index) {
				case 0: {
					value.emplace<sign_suppliers::random>();
					break;
				}
				case 1: {
					auto &sequence = value.emplace<sign_suppliers::sequence>();
					deserializePrimitiveArray(std::span{ sequence.m_colors }, sequence.numColors, src);
					break;
				}
				default: return false;
			}

			return true;
		}

		template<typename It>
		inline bool deserializeScaler(sign_scaler &value, It &src) {
			const auto index = *src++;
			return ztu::for_each::indexed_type<
				sign_scalers::ping_pong,
				sign_scalers::sinus,
				sign_scalers::random,
				sign_scalers::smooth_random
			>([&]<auto Index, typename T>() {
				if (index == Index) {
					value.emplace<Index>();
					return true;
				}
				return false;
			});
		}

		template<typename It>
		inline bool deserializeSequencer(sign_sequencer &value, It &dst) {
			return (
				deserializeSupplier(value.supplier, dst) and
				deserializePrimitive(value.mixType, dst)
			);
		}
	}

	template<typename It>
	inline bool deserialize(sign_animation &value, It &src) {
		auto ok = true;

		auto &animation = value.animator;
		const auto index = *src++;

		switch (index) {
			case 0: {
				auto &uniform_color = animation.emplace<sign_animations::uniform_color>();
				ok = detail::deserializeSequencer(uniform_color.sequencer, src);
				break;
			};
			case 1: {
				auto &moving_colors = animation.emplace<sign_animations::moving_colors>();
				ok = (
					detail::deserializeSequencer(moving_colors.sequencer, src) and
					detail::deserializePrimitive(moving_colors.perPixelOffset, src) and
					detail::deserializePrimitive(moving_colors.pixelOffset, src)
				);
				break;
			};
			case 2: {
				auto &moving_colors = animation.emplace<sign_animations::moving_pixel>();
				ok = (
					detail::deserializeSequencer(moving_colors.sequencer, src) and
					detail::deserializeScaler(moving_colors.scaler, src) and
					detail::deserializePrimitive(moving_colors.colorSpeed, src) and
					detail::deserializePrimitive(moving_colors.width, src)
				);
				break;
			}
			case 3: {
				auto &stop_motion = animation.emplace<sign_animations::stop_motion>();
				detail::deserializePrimitiveArray(std::span{ stop_motion.m_frames }, stop_motion.numFrames, src);
				break;
			}
		}
		return ok and detail::deserializePrimitive(value.speed, src);
	}
}
