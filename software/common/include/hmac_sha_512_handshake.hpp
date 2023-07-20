#pragma once

#include <system_error>
#include <util/uix.hpp>
#include <hmac_sha_512_engine.hpp>
#include <socket_connection.hpp>
#include <fill_random.hpp>

using namespace ztu::uix;

class hmac_sha_512_handshake {
public:
	hmac_sha_512_handshake(hmac_sha_512_engine &n_engine, socket_connection &n_connection);

	[[nodiscard]] std::error_code validate(bool initiator);

private:
	[[nodiscard]] std::error_code challengePeer();

	[[nodiscard]] std::error_code solveChallenge();

private:
	hmac_sha_512_engine &m_engine;
	socket_connection &m_connection;
};
