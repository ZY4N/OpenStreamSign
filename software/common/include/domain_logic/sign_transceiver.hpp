#pragma once

#pragma once

#include <domain_logic/sign_state.hpp>
#include <domain_logic/sign_animation.hpp>

#include <sign_animation_transcoding.hpp>
#include <aes_transceiver.hpp>

enum class sign_message_type : u8 {
	CHANGE_STATE = 0,
	SET_ANIMATION = 1,
	HEARTBEAT = 2
};

namespace sign_messages {

	struct change_state_message {

		static constexpr auto type = sign_message_type::CHANGE_STATE;
		static constexpr usize max_body_size = 0U;

		using data_t = std::tuple<sign_state>;

		struct header_t {
			sign_state state;
			[[nodiscard]] inline u16 bodySize() const {
				return 0;
			}
		};

		inline static bool serialize(header_t& header, std::span<u8>, const sign_state &state) {
			header.state = state;
			return true;
		}

		inline static bool deserialize(const header_t& header, std::span<const u8>, sign_state &state) {
			state = header.state;
			return true;
		}
	};

	struct set_animation_message {

		static constexpr auto type = sign_message_type::SET_ANIMATION;
		// This assumes that sizeof(sign_animation) is smaller or equal to
		// the serialized version.  That's not great, not terrible.
		// Might lead to issues in the future ¯\_(ツ)_/¯.
		static constexpr usize max_body_size = sizeof(sign_animation);

		using data_t = std::tuple<sign_state, sign_animation>;

		struct header_t {
			u16 animationLength;
			sign_state state;
			[[nodiscard]] inline u16 bodySize() const {
				return animationLength;
			}
		};

		inline static bool serialize(
			header_t& header,
			std::span<u8> body,
			const sign_state &state,
			const sign_animation &animation
		) {

			auto it = body.begin();
			*it++ = static_cast<u8>(state);
			sign_animation_transcoding::serialize(animation, it);

			if (it > body.end())
				return false;

			header.animationLength = it - 1 - body.begin();
			header.state = state;

			return true;
		}

		inline static bool deserialize(
			const header_t& header,
			std::span<const u8> body,
			sign_state &state,
			sign_animation &animation
		) {
			state = header.state;
			auto it = body.begin();
			return sign_animation_transcoding::deserialize(animation, it);
		}
	};

	struct heartbeat_message {

		static constexpr auto type = sign_message_type::HEARTBEAT;
		static constexpr usize max_body_size = 0;

		using data_t = std::tuple<>;

		struct header_t {
			[[nodiscard]] inline u16 bodySize() const {
				return 0;
			}
		};

		inline static bool serialize(header_t&, std::span<u8>) {
			return true;
		}

		inline static bool deserialize(const header_t&, std::span<const u8>) {
			return true;
		}
	};
}

using sign_transceiver = aes_transceiver<
	sign_message_type,
	sign_messages::change_state_message,
	sign_messages::set_animation_message,
	sign_messages::heartbeat_message
>;

using sign_message = sign_transceiver::message_t;
