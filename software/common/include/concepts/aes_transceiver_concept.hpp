#pragma once

#include <utility>
#include <concepts>
#include <variant>
#include <tuple>
#include <util/pack.hpp>

namespace aes_transceiver_concepts {

	template<class T>
	concept message_header = (
		std::is_default_constructible_v<T> and
		requires(const T t) {
			{ t.bodySize() } -> std::same_as<unsigned short>;
		}
	);

	template<class T>
	concept message = (
		message_header<typename T::header_t> and
		std::is_enum_v<decltype(T::type)> and
		std::integral<decltype(T::max_body_size)> and
		// static bool T::serialize(header_t& header, std::span<u8>, const Args&... args) and
		requires(typename T::data_t data) {
			{
				std::apply([]<typename... Args>(Args&&... args) {
					typename T::header_t header;
					std::span<const u8> buffer;
					return T::deserialize(header, buffer, std::forward<Args>(args)...);
				}, data)
			} -> std::same_as<bool>;
		}
	);

	template<typename Enum, message... Messages>
		requires (std::is_enum_v<Enum>)
	struct messages_valid {

		static constexpr auto is_enum = std::is_enum_v<Enum>;

		static constexpr auto are_enum_types = (
			std::same_as<std::remove_cvref_t<decltype(Messages::type)>, Enum> and ...
		);

		static constexpr auto are_unique_types = []() {
			using enum_integral = std::underlying_type_t<Enum>;
			auto types = std::array{ static_cast<enum_integral>(Messages::type)... };
			std::sort(types.begin(), types.end());
			return std::adjacent_find(types.begin(), types.end()) == types.end();
		}();

		static_assert(is_enum);
		static_assert(are_enum_types);
		static_assert(are_unique_types);

		static constexpr auto value = is_enum and are_enum_types and are_unique_types;
	};
}
