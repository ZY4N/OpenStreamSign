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
		Enum type() const {
			return static_cast<Enum>(static_cast<enum_int_t>(
				this->index()
			));
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

	static constexpr auto maxHeaderSize		= std::max({ sizeof(typename Messages::header_t)... });
	static constexpr auto maxBodySize		= std::max({ Messages::max_body_size... });
	static constexpr auto maxMessageSize		= aes_256_info::cipherLength(maxHeaderSize) + aes_256_info::cipherLength(maxBodySize);
	static constexpr auto maxPacketSize		= aes_256_info::ivSize + maxMessageSize;

	using messages = ztu::pack<Messages...>;

	using type_integral_t = ztu::uint_holding<sizeof...(Messages)>;

public:
	using message_t = detail::enum_variant<
	    Enum,
	    detail::enum_type<
			Messages::type,
			typename Messages::data_t
		>...
	>;

	aes_transceiver(aes_256_engine &n_engine, socket_connection &n_connection)
		: m_engine{ n_engine }, m_connection{ n_connection } {}

	[[nodiscard]] std::error_code send(const message_t& value);

	template<Enum Type, typename... Args>
		requires (index_of_message<Type, Messages...> < sizeof...(Messages))
	[[nodiscard]] std::error_code send(const Args&... args);

	[[nodiscard]] std::error_code receive(message_t &value);

private:
	[[nodiscard]] std::error_code aes_send(std::span<u8> data);

	[[nodiscard]] std::error_code aes_receive(std::span<u8> data);

private:
	std::array<u8, maxMessageSize> m_buffer{};
	std::array<u8, maxPacketSize> m_aes_buffer{};

	aes_256_engine &m_engine;
	socket_connection &m_connection;
};


#define INCLUDE_AES_TRANSCEIVER_IMPLEMENTATION
#include <aes_transceiver.ipp>
#undef INCLUDE_AES_TRANSCEIVER_IMPLEMENTATION
