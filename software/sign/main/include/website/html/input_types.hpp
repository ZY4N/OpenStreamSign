#pragma once

#include <array>
#include <cinttypes>

#include <util/string_literal.hpp>

using std::size_t;

namespace input_types {

	using ztu::string_literal;

	template<size_t MaxLength>
	struct text {
		static constexpr auto maxBytes = MaxLength;
		using value_t = string_literal<MaxLength + 1>;
		static constexpr auto getAttributes() {
			using namespace ztu::string_literals;
			return "type=text data-t=txt"_sl;
		}
		static constexpr void parseValue(const char *begin, const char *end, value_t &dst) {
			const auto numChars = end - begin;
			std::copy(begin, end, dst.value.begin());
			dst.value[numChars] = '\0';
		}
	};

	template<size_t MaxLength>
	struct password {
		static constexpr auto maxBytes = MaxLength;
		using value_t = string_literal<MaxLength + 1>;
		static constexpr auto getAttributes() {
			using namespace ztu::string_literals;
			return "type=password data-t=txt"_sl;
		}
		static constexpr void parseValue(const char *begin, const char *end, value_t &dst) {
			const auto numChars = end - begin;
			std::copy(begin, end, dst.value.begin());
			dst.value[numChars] = '\0';
		}
	};

	struct button {
		static constexpr auto maxBytes = 0;
		using value_t = bool;
		static constexpr auto getAttributes() {
			using namespace ztu::string_literals;
			return "type=button data-t=ign"_sl;
		}
		static constexpr void parseValue(const char *begin, const char *end, bool &dst) {
			dst = false;
		}
	};

	struct ipv4 {
		static constexpr auto maxBytes = 4;
		using value_t = uint32_t;
		static constexpr auto getAttributes() {
			using namespace ztu::string_literals;
			return "type=text data-t=ipv4"_sl;
		}
		static constexpr auto parseValue(const char *begin, const char *end, value_t &dst) {
			dst = *reinterpret_cast<const value_t*>(begin);
		}
	};

	struct port {
		static constexpr auto maxBytes = 2;
		using value_t = uint16_t;
		static constexpr auto getAttributes() {
			using namespace ztu::string_literals;
			return "type=number min=0 max=65535 step=1 data-t=u16"_sl;
		}
		static constexpr auto parseValue(const char *begin, const char *end, value_t &dst) {
			dst = *reinterpret_cast<const value_t*>(begin);
		}
	};
}
