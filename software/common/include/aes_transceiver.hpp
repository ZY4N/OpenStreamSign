#pragma once

#include <error_codes/aes_transceiver_error.hpp>
#include <concepts/aes_transceiver_concept.hpp>
#include <aes_256_info.hpp>

#include <socket_connection.hpp>
#include <aes_256_engine.hpp>
#include <fill_random.hpp>

#include <util/pack.hpp>
#include <util/uix.hpp>
#include <algorithm>
#include <variant>
#include <tuple>

using namespace ztu::uix;

template<auto Type, aes_transceiver_concepts::message... Messages>
inline constexpr auto index_of_message = (
	ztu::pack<Messages...>::template index_of_f([]<class Message>() {
		return Type == Message::type;
	})
);


namespace detail {

	template<auto Type, typename T>
	struct enum_type {
		static constexpr auto enum_v = Type;
		using enum_t = std::remove_cvref_t<decltype(Type)>;
		using type = T;
	};

	template<typename T>
	concept enum_type_concept = (
		std::same_as<std::void_t<typename T::type>, void> and
		std::same_as<std::remove_cvref_t<decltype(T::enum_v)>, typename T::enum_t>
	);

	template<typename Enum, enum_type_concept... Alternatives>
		requires (
			std::is_enum_v<Enum> and
			(std::same_as<Enum, typename Alternatives::enum_t> and ...)
		)
	class enum_variant : public std::variant<typename Alternatives::type...> {
		using base_t = std::variant<typename Alternatives::type...>;
		using enum_int_t = std::underlying_type_t<Enum>;
		using alternatives = ztu::pack<Alternatives...>;

	public:
		enum_variant() = default;

		Enum type() const {
			const auto currentIndex = this->index();
			Enum currentType{};
			alternatives::template indexed_for_each([&]<auto Index, typename Alternative>() {
				if (currentIndex == Index) {
					currentType = Alternative::enum_v;
					return true;
				}
				return false;
			});
			return currentType;
		}

		template<Enum Type>
		decltype(auto) get() {
			const auto index = alternatives::template index_of_f([]<typename Alternative>() {
				return Type == Alternative::type;
			});
			static_assert(index < alternatives::size);
			return std::get<index>(*reinterpret_cast<base_t*>(this));
		}

		template<Enum Type>
		decltype(auto) get() const {
			const auto index = alternatives::template index_of_f([]<typename Alternative>() {
				return Type == Alternative::enum_v;
			});
			static_assert(index < alternatives::size);
			return std::get<index>(*reinterpret_cast<const base_t*>(this));
		}
	};
}

template<typename Enum, aes_transceiver_concepts::message... Messages>
	requires (aes_transceiver_concepts::messages_valid<Enum, Messages...>::value)
struct aes_transceiver {
private:

	using type_integral_t = ztu::uint_holding<sizeof...(Messages)>;

	static constexpr auto max_meta_size				= std::max({ sizeof(typename Messages::meta_t)... });
	static constexpr auto max_header_size			= sizeof(type_integral_t) + max_meta_size;
	static constexpr auto max_body_size				= std::max({ Messages::max_body_size... });
	static constexpr auto max_header_packet_size	= aes_256_info::ivSize + aes_256_info::cipherLength(max_header_size);
	static constexpr auto max_body_packet_size		= aes_256_info::ivSize + aes_256_info::cipherLength(max_header_size);
	static constexpr auto text_buffer_size			= max_header_size + max_body_size;
	static constexpr auto packet_buffer_size		= max_header_packet_size + max_body_packet_size;

	using messages = ztu::pack<Messages...>;

public:
	using header_t = detail::enum_variant<
	    Enum,
	    detail::enum_type<
			Messages::type,
			typename Messages::meta_t
		>...
	>;

	using message_t = detail::enum_variant<
	    Enum,
	    detail::enum_type<
			Messages::type,
			typename Messages::data_t
		>...
	>;

	aes_transceiver() = default;

	[[nodiscard]] aes_256_engine*& engine() { return m_engine; }

	[[nodiscard]] std::error_code encrypt_message(std::span<u8> &packet, const message_t& message);

	template<Enum Type, typename... Args>
		requires (index_of_message<Type, Messages...> < sizeof...(Messages))
	[[nodiscard]] std::error_code encrypt_message(std::span<u8> &packet, const Args&... args);

	std::span<u8> header_packet_buffer();

	[[nodiscard]] std::error_code decrypt_header(
		header_t &header, std::span<u8> &body_packet_buffer
	);

	[[nodiscard]] std::error_code decrypt_body(
		const header_t &header, message_t& message
	);

private:
	[[nodiscard]] std::error_code encrypt(std::span<u8> text_buffer, usize text_size, std::span<u8> &packet);

	[[nodiscard]] std::error_code decrypt(std::span<u8> packet, std::span<u8> &text_buffer);

private:
	std::array<u8, text_buffer_size> m_text_buffer{};
	std::array<u8, packet_buffer_size> m_packet_buffer{};

	aes_256_engine* m_engine{ nullptr };
};


#define INCLUDE_AES_TRANSCEIVER_IMPLEMENTATION
#include <aes_transceiver.ipp>
#undef INCLUDE_AES_TRANSCEIVER_IMPLEMENTATION
