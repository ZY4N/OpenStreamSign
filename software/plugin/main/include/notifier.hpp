#pragma once


#include <config/notifier_config.hpp>
#include <domain_logic/sign_transceiver.hpp>
#include <hmac_sha_512_handshake.hpp>

class notifier {
public:
	notifier(const notifier_config_t& cfg);

	[[nodiscard]] std::error_code connect();

	void changeState(const sign_state &newState);

	void disconnect();

	~notifier();

private:
	[[nodiscard]] std::error_code handleHandshake();

	void onConnect();

	void sendDecoyCommands();

	template<sign_message_type Type, typename... Args>
	void sendMessage(Args&&... args);

private:
	const notifier_config_t &config;

	socket_connection connection{};
	aes_256_engine aes_engine{};
	hmac_sha_512_engine sha_engine{};
	sign_transceiver transceiver{ aes_engine, connection };
	hmac_sha_512_handshake validator{ sha_engine, connection };

	std::array<uint8_t, 64 + 32> secret;

	std::atomic<sign_state> state{ sign_state::IDLE };

	std::thread connectingThread;
	std::mutex intervalMutex, connectionMutex;
	std::condition_variable intervalDisruptor;
	std::atomic_bool connected{ false }, reconnect{ true };
};