#pragma once

#include <system_error>

namespace hmac_sha_512_handshake_error {
	enum class codes {
		OK,
		PEER_COULD_NOT_SOLVE_CHALLENGE,
		COULD_NOT_SOLVE_CHALLENGE
	};

	struct category : std::error_category {
		const char* name() const noexcept override {
			return "handshake";
		}
		std::string message(int ev) const override {
			switch (static_cast<codes>(ev)) {
				using enum codes;
			case PEER_COULD_NOT_SOLVE_CHALLENGE:
				return "Peer sent wrong handshake hash";
			case COULD_NOT_SOLVE_CHALLENGE:
				return "Could not solve the challenge received from peer";
			default:
				return "(unrecognized error)";
			}
		}
	};
}

inline hmac_sha_512_handshake_error::category& hmac_sha_512_handshake_category() {
	static hmac_sha_512_handshake_error::category cat;
	return cat;
}

namespace hmac_sha_512_handshake_error {
	inline std::error_code make_error_code(codes e) {
		return { static_cast<int>(e), hmac_sha_512_handshake_category() };
	}
}

template <>
struct std::is_error_code_enum<hmac_sha_512_handshake_error::codes> : public std::true_type {};
