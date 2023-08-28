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

		struct meta_t {
			sign_state state;
			[[nodiscard]] inline u16 body_size() const {
				return 0;
			}
		};

		inline static bool serialize(meta_t& meta, std::span<u8>, const sign_state &state) {
			meta.state = state;
			return true;
		}

		inline static bool deserialize(const meta_t& meta, std::span<const u8>, sign_state &state) {
			state = meta.state;
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

		struct meta_t {
			u16 animationLength;
			sign_state state;
			[[nodiscard]] inline u16 body_size() const {
				return animationLength;
			}
		};

		inline static bool serialize(
			meta_t& meta,
			std::span<u8> body,
			const sign_state &state,
			const sign_animation &animation
		) {

			auto it = body.begin();
			*it++ = static_cast<u8>(state);
			sign_animation_transcoding::serialize(animation, it);

			if (it > body.end())
				return false;

			meta.animationLength = it - 1 - body.begin();
			meta.state = state;

			return true;
		}

		inline static bool deserialize(
			const meta_t& meta,
			std::span<const u8> body,
			sign_state &state,
			sign_animation &animation
		) {
			state = meta.state;
			auto it = body.begin();
			return sign_animation_transcoding::deserialize(animation, it);
		}
	};

	struct heartbeat_message {

		static constexpr auto type = sign_message_type::HEARTBEAT;
		static constexpr usize max_body_size = 0;

		using data_t = std::tuple<>;

		struct meta_t {
			[[nodiscard]] inline u16 body_size() const {
				return 0;
			}
		};

		inline static bool serialize(meta_t&, std::span<u8>) {
			return true;
		}

		inline static bool deserialize(const meta_t&, std::span<const u8>) {
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

using sign_header = sign_transceiver::header_t;
using sign_message = sign_transceiver::message_t;
