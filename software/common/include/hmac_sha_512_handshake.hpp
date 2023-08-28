#pragma once

#include <system_error>
#include <util/uix.hpp>
#include <hmac_sha_512_engine.hpp>
#include <socket_connection.hpp>
#include <fill_random.hpp>

using namespace ztu::uix;

namespace hmac_sha_512_handshake {
	namespace detail {
		[[nodiscard]] std::error_code challengePeer(
			hmac_sha_512_engine &engine, socket_connection &connection
		);
		[[nodiscard]] std::error_code solveChallenge(
			hmac_sha_512_engine &engine, socket_connection &connection
		);
	}

	[[nodiscard]] std::error_code validate(
		hmac_sha_512_engine &engine, socket_connection &connection,
		bool initiator
	);
};
