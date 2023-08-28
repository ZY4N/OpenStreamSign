#pragma once

#include <system_error>

namespace aes_transceiver_error {
	enum class codes {
		OK,
		INVALID_MESSAGE_TYPE,
		INVALID_MESSAGE_SIZE,
		SERIALIZATION_ERROR,
		DESERIALIZATION_ERROR
	};

	struct category : std::error_category {
		[[nodiscard]] const char* name() const noexcept override {
			return "aes_transceiver";
		}
		[[nodiscard]] std::string message(int ev) const override {
			switch (static_cast<codes>(ev)) {
				using enum codes;
			case INVALID_MESSAGE_TYPE:
				return "Received header with invalid message type";
			case INVALID_MESSAGE_SIZE:
				return "Received header with invalid message size";
			case SERIALIZATION_ERROR:
				return "The serialization function returned an error";
			case DESERIALIZATION_ERROR:
				return "The deserialization function returned an error";
			default:
				return "(unrecognized error)";
			}
		}
	};
}

inline aes_transceiver_error::category& aes_transceiver_category() {
	static aes_transceiver_error::category cat;
	return cat;
}

namespace aes_transceiver_error {
	inline std::error_code make_error_code(codes e) {
		return { static_cast<int>(e), aes_transceiver_category() };
	}
}

template <>
struct std::is_error_code_enum<aes_transceiver_error::codes> : public std::true_type {};

