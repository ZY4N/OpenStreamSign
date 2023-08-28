#include <hmac_sha_512_handshake.hpp>
#include <error_codes/hmac_sha_512_handshake_error.hpp>

std::error_code hmac_sha_512_handshake::detail::challengePeer(
	hmac_sha_512_engine &engine, socket_connection &connection
) {
	using hmac_sha_512_handshake_error::make_error_code;
	using enum hmac_sha_512_handshake_error::codes;

	std::array<uint8_t, 64> challenge, hash, answer;
	fill_random(challenge);

	std::error_code e;

	if ((e = engine.hash(challenge, hash)))
		return e;

	if ((e = connection.send(challenge)))
		return e;

	if ((e = connection.receive(answer)))
		return e;

	const std::array<uint8_t, 1> correct{hash == answer};

	if ((e = connection.send(correct)))
		return e;

	if (correct[0]) {
		return make_error_code(OK);
	} else {
		return make_error_code(PEER_COULD_NOT_SOLVE_CHALLENGE);
	}
}


std::error_code hmac_sha_512_handshake::detail::solveChallenge(
	hmac_sha_512_engine &engine, socket_connection &connection
) {
	using hmac_sha_512_handshake_error::make_error_code;
	using enum hmac_sha_512_handshake_error::codes;

	std::array<unsigned char, 64> challenge, hash;

	std::error_code e;

	if ((e = connection.receive(challenge)))
		return e;

	if ((e = engine.hash(challenge, hash)))
		return e;

	if ((e = connection.send(hash)))
		return e;

	std::array<uint8_t, 1> correct;

	if ((e = connection.receive(correct)))
		return e;

	if (correct[0]) {
		return make_error_code(OK);
	} else {
		return make_error_code(COULD_NOT_SOLVE_CHALLENGE);
	};
}


[[nodiscard]] std::error_code hmac_sha_512_handshake::validate(
	hmac_sha_512_engine &engine, socket_connection &connection,
	const bool initiator
) {
	const auto executeActions = [&](auto&&... actions) {
		std::error_code error;
		((error = actions(engine, connection), not error) and ...);
		return error;
	};
	auto actions = std::pair{
		&detail::challengePeer,
		&detail::solveChallenge
	};
	return (initiator ?
		executeActions(actions.first, actions.second) :
		executeActions(actions.second, actions.first)
	);
}
