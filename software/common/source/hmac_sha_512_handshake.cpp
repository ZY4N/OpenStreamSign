#include <hmac_sha_512_handshake.hpp>
#include <error_codes/hmac_sha_512_handshake_error.hpp>


hmac_sha_512_handshake::hmac_sha_512_handshake(hmac_sha_512_engine &n_engine, socket_connection &n_connection)
	: m_engine{ n_engine }, m_connection{ n_connection } {}

std::error_code hmac_sha_512_handshake::challengePeer() {
	using hmac_sha_512_handshake_error::make_error_code;
	using enum hmac_sha_512_handshake_error::codes;

	std::array<uint8_t, 64> challenge, hash, answer;
	fill_random(challenge);

	std::error_code e;

	if ((e = m_engine.hash(challenge, hash)))
		return e;

	if ((e = m_connection.send(challenge)))
		return e;

	if ((e = m_connection.receive(answer)))
		return e;

	const std::array<uint8_t, 1> correct{hash == answer};

	if ((e = m_connection.send(correct)))
		return e;

	if (correct[0]) {
		return make_error_code(OK);
	} else {
		return make_error_code(PEER_COULD_NOT_SOLVE_CHALLENGE);
	}
}


std::error_code hmac_sha_512_handshake::solveChallenge() {
	using hmac_sha_512_handshake_error::make_error_code;
	using enum hmac_sha_512_handshake_error::codes;

	std::array<unsigned char, 64> challenge, hash;

	std::error_code e;

	if ((e = m_connection.receive(challenge)))
		return e;

	if ((e = m_engine.hash(challenge, hash)))
		return e;

	if ((e = m_connection.send(hash)))
		return e;

	std::array<uint8_t, 1> correct;

	if ((e = m_connection.receive(correct)))
		return e;

	if (correct[0]) {
		return make_error_code(OK);
	} else {
		return make_error_code(COULD_NOT_SOLVE_CHALLENGE);
	};
}


[[nodiscard]] std::error_code hmac_sha_512_handshake::validate(const bool initiator) {
	const auto executeActions = [&](auto&&... actions) {
		std::error_code error;
		((error = actions(), not error) and ...);
		return error;
	};
	auto actions = std::pair{
		[this]() { return challengePeer(); },
		[this]() { return solveChallenge(); }
	};
	return (initiator ?
		executeActions(actions.first, actions.second) :
		executeActions(actions.second, actions.first)
	);
}
