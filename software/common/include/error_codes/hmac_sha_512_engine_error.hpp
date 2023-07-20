#pragma once

#include <system_error>

namespace hmac_sha_512_engine_error {
	enum class codes {
		OK,
		CONTEXT_INITIALIZATION_FAILED,
		WRONG_KEY_SIZE,
		WRONG_MESSAGE_SIZE,
		WRONG_HASH_SIZE,
		USE_BEFORE_INITIALIZATION,
		INTERNAL_ERROR
	};

	struct category : std::error_category {
		const char* name() const noexcept override {
			return "hmacSHA512";
		}
		std::string message(int ev) const override {
			switch (static_cast<codes>(ev)) {
				using enum codes;
			case WRONG_KEY_SIZE:
				return "Key size is not of size 512 bits";
			case CONTEXT_INITIALIZATION_FAILED:
				return "Initialization of the implementation dependent context failed";
			case USE_BEFORE_INITIALIZATION:
				return "hash function used before succesfull initialization";
			case WRONG_MESSAGE_SIZE:
				return "message size is wrong";
			case WRONG_HASH_SIZE:
				return "hash size is wrong";
			case INTERNAL_ERROR:
				return "unexpected internal error";
			default:
				return "(unrecognized error)";
			}
		}
	};

	inline std::error_code make_error_code(codes e) {
		static category cat;
		return { static_cast<int>(e), cat };
	}
}

template <>
struct std::is_error_code_enum<hmac_sha_512_engine_error::codes> : public std::true_type {};
