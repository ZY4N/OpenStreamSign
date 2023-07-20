#ifndef INCLUDE_AES_TRANSCEIVER_IMPLEMENTATION
#error Never include this file directly include 'aes_transceiver.hpp'
#endif

#include <cassert>
#include <algorithm>


template<typename Enum, aes_transceiver_concepts::message... Messages>
	requires (aes_transceiver_concepts::messages_valid<Enum, Messages...>::value)
std::error_code aes_transceiver<Enum, Messages...>::send(const message_t& value) {
	const auto index = value.index();
	std::error_code error;
	messages::indexed_for_each_f([&]<auto Index, class Message>() {
		if (index == Index) {
			error = send<Message::type>(std::get<Index>(value));
			return true;
		}
		return false;
	});
	return error;
}


template<typename Enum, aes_transceiver_concepts::message... Messages>
	requires (aes_transceiver_concepts::messages_valid<Enum, Messages...>::value)
template<Enum Type, typename... Args>
	requires (index_of_message<Type, Messages...> < sizeof...(Messages))
[[nodiscard]] std::error_code aes_transceiver<Enum, Messages...>::send(const Args&... args) {

	using enum aes_transceiver_error::codes;
	using namespace aes_transceiver_error;

	constexpr auto index = index_of_message<Type, Messages...>;
	using message = messages::template at<index>;
	using header_t = message::header_t;

	auto typeBuffer = std::span{ m_buffer.begin(), sizeof(type_integral_t) };
	auto headerBuffer = std::span{ typeBuffer.end(), sizeof(header_t) };
	auto bodyBuffer = std::span{ headerBuffer.end(), message::max_body_size };

	constexpr auto typeIndex = static_cast<type_integral_t>(index);
	const auto &typeBytes = *reinterpret_cast<const std::array<u8, sizeof(type_integral_t)>*>(&typeIndex);
	std::copy(typeBytes.begin(), typeBytes.end(), typeBuffer.begin());

	auto &header = *reinterpret_cast<header_t*>(headerBuffer.data());

	if (not message::serialize(header, bodyBuffer, std::forward<const Args>(args)...))
		return make_error_code(SERIALIZATION_ERROR);

	std::error_code error;
	if ((error = aes_send({ typeBuffer.begin(), headerBuffer.end() })))
		return error;

	if ((error = aes_send({ bodyBuffer.begin(), header.bodySize() })))
		return error;

	return OK;
}

template<typename Enum, aes_transceiver_concepts::message... Messages>
	requires (aes_transceiver_concepts::messages_valid<Enum, Messages...>::value)
std::error_code aes_transceiver<Enum, Messages...>::receive(message_t &value) {
	using enum aes_transceiver_error::codes;

	const auto typeBuffer = std::span{ m_buffer.begin(), sizeof(type_integral_t) };
	const auto headerBuffer = std::span{ typeBuffer.end(), maxHeaderSize };

	std::error_code error;
	if ((error = aes_receive({ typeBuffer.begin(), headerBuffer.end() })))
		return error;

	const auto index = *reinterpret_cast<const type_integral_t*>(typeBuffer.data());

	error = INVALID_MESSAGE_TYPE;

	messages::indexed_for_each(
		[&]<auto Index, typename Message>() {
			if (Index == index) {

				const auto header = *reinterpret_cast<const typename Message::header_t*>(headerBuffer.data());
		
				const auto bodySize = header.bodySize();
				const auto spaceLeft = &(*m_buffer.end()) - &(*headerBuffer.end());
				if (bodySize > spaceLeft) {
					error = INVALID_MESSAGE_SIZE;
					return true;
				}

				const auto bodyBuffer = std::span{ headerBuffer.end(), bodySize };
				if ((error = aes_receive(bodyBuffer))) {
					return true;
				}

				auto &data = value.template emplace<Index>();
				const auto ok = std::apply([&](auto&... args) {
					return Message::deserialize(header, bodyBuffer, args...);
				}, data);

				error = ok ? OK : DESERIALIZATION_ERROR;

				return true;
			}
			return false;
		}
	);

	return error;
}

template<typename Enum, aes_transceiver_concepts::message... Messages>
	requires (aes_transceiver_concepts::messages_valid<Enum, Messages...>::value)
std::error_code aes_transceiver<Enum, Messages...>::aes_send(std::span<u8> data) {
	using enum aes_transceiver_error::codes;

	const auto cipherLength = aes_256_info::cipherLength(data.size());

	// plaintextBuffer is needed because pck7 padding is added in place
	// into the plaintext before encrypting to the body.
	const auto plainTextBuffer = std::span{ data.begin(), cipherLength };
	const auto iv = std::span{ m_aes_buffer.begin(), aes_256_info::ivSize };
	const auto body = std::span{ iv.end(), cipherLength };
	const auto packet = std::span{ iv.begin(), body.end() };

	fill_random(iv);

	std::error_code error; usize actualCipherLength;
	if ((error = m_engine.encrypt(iv, plainTextBuffer, data.size(), body, actualCipherLength)))
		return error;

	// assert(cipherLength == actualCipherLength);

	if ((error = m_connection.send(packet)))
		return error;

	return OK;
}

template<typename Enum, aes_transceiver_concepts::message... Messages>
	requires (aes_transceiver_concepts::messages_valid<Enum, Messages...>::value)
std::error_code aes_transceiver<Enum, Messages...>::aes_receive(std::span<u8> data) {
	using enum aes_transceiver_error::codes;

	const auto cipherLength = aes_256_info::cipherLength(data.size());

	const auto plainTextBuffer = std::span{ data.begin(), cipherLength };
	const auto iv = std::span{ m_aes_buffer.begin(), aes_256_info::ivSize };
	const auto body = std::span{ iv.end(), cipherLength };
	const auto packet = std::span{ iv.begin(), body.end() };

	std::error_code error;
	if ((error = m_connection.receive(packet)))
		return error;

	usize plainTextSize;
	if ((error = m_engine.decrypt(iv, body, plainTextBuffer, plainTextSize)))
		return error;

	return OK;
}
