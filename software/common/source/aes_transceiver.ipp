#ifndef INCLUDE_AES_TRANSCEIVER_IMPLEMENTATION
#error Never include this file directly include 'aes_transceiver.hpp'
#endif

#include <cassert>
#include <algorithm>


template<typename Enum, aes_transceiver_concepts::message... Messages>
	requires (aes_transceiver_concepts::messages_valid<Enum, Messages...>::value)
template<Enum Type, typename... Args>
		requires (index_of_message<Type, Messages...> < sizeof...(Messages))
std::error_code aes_transceiver<Enum, Messages...>::encrypt_message(
	std::span<u8> &packet, const Args&... args
) {
	using enum aes_transceiver_error::codes;
	using namespace aes_transceiver_error;

	constexpr auto index = index_of_message<Type, Messages...>;
	using message = messages::template at<index>;
	using meta_t = message::meta_t;

	const auto text_buffer_view = std::span<u8>{ m_text_buffer };
	const auto packet_buffer_view = std::span<u8>{ m_packet_buffer };

	usize header_size = 0;
	auto type_buffer	= text_buffer_view.subspan(header_size, sizeof(type_integral_t));
	header_size		  += type_buffer.size();
	auto meta_buffer	= text_buffer_view.subspan(header_size, sizeof(meta_t));
	header_size		  += meta_buffer.size();

	auto padded_header_size	= aes_256_info::cipherLength(header_size); // leave space for pkcs7
	auto padded_body_buffer = text_buffer_view.subspan(padded_header_size, message::max_body_size);

	meta_t meta;

	if (not message::serialize(meta, padded_body_buffer, std::forward<const Args>(args)...))
		return make_error_code(SERIALIZATION_ERROR);

	constexpr auto type_index = static_cast<type_integral_t>(index);
	std::copy_n(reinterpret_cast<const u8*>(&type_index), sizeof(type_integral_t), type_buffer.begin());
	std::copy_n(reinterpret_cast<const u8*>(&meta      ), sizeof(meta_t         ), meta_buffer.begin());

	std::error_code error;
	const auto padded_header_buffer = text_buffer_view.subspan(0, padded_header_size);
	auto header_packet = packet_buffer_view;
	if ((error = encrypt(padded_header_buffer, header_size, header_packet)))
		return error;

	const auto body_size = meta.body_size();
	auto body_packet = packet_buffer_view.subspan(header_packet.size());
	if ((error = encrypt(padded_body_buffer, body_size, body_packet)))
		return error;

	packet = { header_packet.begin(), body_packet.end() };
	
	return make_error_code(OK);
}

template<typename Enum, aes_transceiver_concepts::message... Messages>
	requires (aes_transceiver_concepts::messages_valid<Enum, Messages...>::value)
std::span<u8> aes_transceiver<Enum, Messages...>::header_packet_buffer() {
	return { m_packet_buffer.begin(), max_header_packet_size };
}

template<typename Enum, aes_transceiver_concepts::message... Messages>
	requires (aes_transceiver_concepts::messages_valid<Enum, Messages...>::value)
std::error_code aes_transceiver<Enum, Messages...>::decrypt_header(
	header_t &header, std::span<u8> &body_packet_buffer
) {
	using enum aes_transceiver_error::codes;
	using namespace aes_transceiver_error;

	auto header_buffer = std::span<u8>{ m_text_buffer };
	
	std::error_code error;
	if ((error = decrypt(header_packet_buffer(), header_buffer)))
		return error;

	const auto type_index = *reinterpret_cast<const type_integral_t*>(header_buffer.data());

	error = INVALID_MESSAGE_TYPE;

	messages::indexed_for_each([&]<auto Index, typename Message>() {
		if (Index == type_index) {
			
			using meta_t = Message::meta_t;
			const auto meta = *reinterpret_cast<const meta_t*>(header_buffer.data() + sizeof(type_index));
			const auto body_size = meta.body_size();
			const auto body_packet_size = aes_256_info::ivSize + aes_256_info::cipherLength(body_size);

			if (body_packet_size > max_body_packet_size) {
				error = INVALID_MESSAGE_SIZE;
			} else {
				error = OK;
				header.template emplace<Index>(meta);
				body_packet_buffer = { m_packet_buffer.begin(), body_packet_size };
			}

			return true;
		}
		return false;
	});

	return error;
}

template<typename Enum, aes_transceiver_concepts::message... Messages>
	requires (aes_transceiver_concepts::messages_valid<Enum, Messages...>::value)
std::error_code aes_transceiver<Enum, Messages...>::decrypt_body(
	const header_t &header, message_t& message
) {
	using enum aes_transceiver_error::codes;
	using namespace aes_transceiver_error;
	
	const auto type_index = header.index();

	std::error_code error = INVALID_MESSAGE_TYPE;

	messages::indexed_for_each(
		[&]<auto Index, typename Message>() {
			if (Index == type_index) {

				const auto& meta = *std::get_if<Index>(&header);

				const auto body_size = meta.body_size();
				const auto body_packet_size = aes_256_info::ivSize + aes_256_info::cipherLength(body_size);
				const auto body_packet_buffer = std::span<u8>{ m_packet_buffer.begin(), body_packet_size };

				auto body_buffer = std::span<u8>{ m_text_buffer };
				if ((error = decrypt(body_packet_buffer, body_buffer)))
					return true;
			
				auto &data = message.template emplace<Index>();
				const auto ok = std::apply([&](auto&... args) {
					return Message::deserialize(meta, body_buffer, args...);
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
std::error_code aes_transceiver<Enum, Messages...>::encrypt(std::span<u8> text_buffer, usize text_size, std::span<u8> &packet) {
	using enum aes_transceiver_error::codes;

	const auto cipher_size = aes_256_info::cipherLength(text_size);

	// Shrink text buffer to required size
	text_buffer = text_buffer.subspan(0, cipher_size);

	const auto packet_buffer_view = std::span<u8>{ m_packet_buffer };

	auto offset		= packet.data() - packet_buffer_view.data();
	const auto iv	= packet_buffer_view.subspan(offset, aes_256_info::ivSize);
	offset		  += iv.size();
	auto cipher		= packet_buffer_view.subspan(offset, cipher_size);

	fill_random(iv);

	std::error_code error; usize actual_cipherLength;
	if ((error = m_engine->encrypt(iv, text_buffer, text_size, cipher, actual_cipherLength)))
		return error;
		
	assert(cipher.size() == actual_cipherLength);

	packet = { iv.begin(), cipher.end() };

	return OK;
}

template<typename Enum, aes_transceiver_concepts::message... Messages>
	requires (aes_transceiver_concepts::messages_valid<Enum, Messages...>::value)
std::error_code aes_transceiver<Enum, Messages...>::decrypt(std::span<u8> packet, std::span<u8> &text_buffer) {
	using enum aes_transceiver_error::codes;

	const auto iv		= std::span<u8>{ packet.begin(), aes_256_info::ivSize };
	const auto cipher	= std::span<u8>{ iv.end(), packet.end() };

	std::error_code error; usize text_size;
	if ((error = m_engine->decrypt(iv, cipher, text_buffer, text_size)))
		return error;

	assert(aes_256_info::cipherLength(text_size) == cipher.size());
	
	text_buffer = text_buffer.subspan(0, text_size);
	
	return OK;
}
