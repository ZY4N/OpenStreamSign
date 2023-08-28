#pragma once


#include <config/app_config.hpp>
#include <domain_logic/sign_transceiver.hpp>
#include <hmac_sha_512_handshake.hpp>

class app {
public:
	app() = default;

	[[nodiscard]] std::error_code start();

	void changeState(const sign_state &newState);

	~app();

private:

	void loadConfig(const std::string_view &configFileName);

	[[nodiscard]] std::error_code initEncryptionEngines();

	void handleConnection();

	[[nodiscard]] std::error_code validateConnection();

	void onConnect();

	void sendDecoyCommands();

	template<sign_message_type Type, typename... Args>
	void sendMessage(Args&&... args);

	void disconnect();

private:
	app_config_t config;

	std::atomic<sign_state> state{ sign_state::IDLE };

	std::array<uint8_t, 64 + 32> secret;
	socket_connection connection{};
	aes_256_engine aes_engine{};
	hmac_sha_512_engine sha_engine{};
	sign_transceiver transceiver{};

	std::thread connectionThread;
	std::mutex intervalMutex, connectionMutex;
	std::condition_variable intervalDisruptor;
	std::atomic_bool connected{ false }, reconnect{ true };
};