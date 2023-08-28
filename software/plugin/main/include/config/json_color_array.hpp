#pragma once

#include <platform/safe_json.hpp>
#include <util/uix.hpp>
#include <lighting/color.hpp>
#include <vector>
#include <util/for_each.hpp>

namespace color_array_detail {
	using namespace json::safe;
	using namespace default_values;

	struct string_color_array_converter {
		using x_t = string_t;
		using y_t = std::vector<color>;

		static std::optional<y_t> convert(const x_t &x) {

			std::vector<color> colors;
			colors.reserve(x.length() / 4);

			u8 numHexChars = 0;
			std::array<u8, 6> buffer{};

			const auto pushColor = [&]() {
				switch (numHexChars) {
				case 6:
					colors.emplace_back(
						static_cast<u8>(16 * buffer[0] + buffer[1]),
						static_cast<u8>(16 * buffer[2] + buffer[3]),
						static_cast<u8>(16 * buffer[4] + buffer[5])
					);
					break;
				case 3:
					colors.emplace_back(
						static_cast<u8>(17 * buffer[0]),
						static_cast<u8>(17 * buffer[1]),
						static_cast<u8>(17 * buffer[2])
					);
					break;
				default:
					return false;
				}

				std::fill(buffer.begin(), buffer.end(), 0);
				numHexChars = 0;

				return true;
			};

			for (const char c : x) {
				u8 byte = 0x80;
				if ('0' <= c && c <= '9') {
					byte = c - '0';
				} else if ('a' <= c && c <= 'f') {
					byte = 10 + c - 'a';
				} else if ('A' <= c && c <= 'F') {
					byte = 10 + c - 'A';
				}

				if (byte & 0x80) {
					if (not (c == ' ' and pushColor())) {
						return std::nullopt;
					}
				} else if (numHexChars == 6) {
					return std::nullopt;
				} else {
					buffer[numHexChars++] = byte;
				}
			}

			std::optional<y_t> y;

			if (pushColor()) {
				y.emplace(std::move(colors));
			}

			return y;
		};

		static std::optional<x_t> revert(const y_t &y) {

			x_t x{};
			x.reserve(y.size() * 4);

			for (const auto &[r, g, b] : y) {
				std::array<std::pair<u8,u8>, 3> digits;

				bool shortNotation = true;
				ztu::for_each::indexed_argument(
					[&]<size_t Index>(const auto &channel) {
						auto &digit = digits[Index];
						digit.first = channel / 16;
						digit.second = channel - 16 * digit.first;
						shortNotation &= digit.second == 0;
						return false;
					},
					r, g, b
				);

				const auto hexChar = [](const u8 num) {
					return static_cast<char>(num < 10 ? '0' + num : 'a' + num - 10);
				};

				if (shortNotation) {
					ztu::for_each::index<3>([&]<size_t Index>() {
						x += hexChar(digits[Index].first);
						return false;
					});
				} else {
					ztu::for_each::index<3>([&]<size_t Index>() {
						x += hexChar(digits[Index].first);
						x += hexChar(digits[Index].second);
						return false;
					});
				}

				x += ' '; // additional space gets removed later
			}

			if (not x.empty()) {
				x.resize(x.size() - 1);
			}

			return x;
		};
	};

	template<std::size_t StrLen>
	using string_color_array_adapter = adapter<string<StrLen>, std::vector<color>, string_color_array_converter>;


	template<string DefaultValue>
	struct color_array : string_color_array_adapter<DefaultValue.size() + 1> {
		constexpr color_array() : string_color_array_adapter<DefaultValue.size() + 1>{ DefaultValue }{}
	};
}

namespace json::safe::default_values {
	template<string DefaultValue>
	struct registerDefaultValue<color_array_detail::color_array<DefaultValue>> : public type_container<ADAPTER> {};
}

template<color_array_detail::string DefaultValue>
using color_array = color_array_detail::color_array<DefaultValue>;
