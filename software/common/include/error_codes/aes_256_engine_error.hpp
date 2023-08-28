#pragma once

#include <system_error>

namespace aes_256_engine_error {
	enum class codes {
		OK,
		WRONG_KEY_SIZE,
		CONTEXT_INITIALIZATION_FAILED,
		INVALID_IV_SIZE,
		PLAIN_TEXT_BUFFER_TOO_SMALL,
		CIPHER_TEXT_BUFFER_TOO_SMALL,
		CORRUPT_PKC7_PADDING,
		USE_BEFORE_INITIALIZATION,
		INTERNAL_ERROR
	};

	struct category : std::error_category {
		const char* name() const noexcept override {
			return "aes256";
		}
		std::string message(int ev) const override {
			switch (static_cast<codes>(ev)) {
				using enum codes;
			case WRONG_KEY_SIZE:
				return "The given key is not of size 256 bits";
			case CONTEXT_INITIALIZATION_FAILED:
				return "Initialization of the implementation dependent context failed";
			case INVALID_IV_SIZE:
				return "The given initialization vector ist not of the correct size";
			case PLAIN_TEXT_BUFFER_TOO_SMALL:
				return "Given plain text buffer is too small to hold text and or padding";
			case CIPHER_TEXT_BUFFER_TOO_SMALL:
				return "Given cipher text buffer is too small";
			case CORRUPT_PKC7_PADDING:
				return "PKC#7 padding corrupted";
			case USE_BEFORE_INITIALIZATION:
				return "Use before succesfull initialization";
			case INTERNAL_ERROR:
				return "unexpected internal error";
			default:
				return "(unrecognized error)";
			}
		}
	};
}

inline aes_256_engine_error::category&	aes_256_engine_category() {
	static aes_256_engine_error::category cat;
	return cat;
}

namespace aes_256_engine_error {
	inline std::error_code make_error_code(codes e) {
		return { static_cast<int>(e), aes_256_engine_category() };
	}
}

template <>
struct std::is_error_code_enum<aes_256_engine_error::codes> : public std::true_type {};
