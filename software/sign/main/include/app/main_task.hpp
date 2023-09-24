#pragma once

#include <platform/wifi_access_point_handler.hpp>
#include <website/config_webserver.hpp>
#include <platform/lwip_socket_acceptor.hpp>
#include <platform/wifi_client_handler.hpp>

#include <hmac_sha_512_handshake.hpp>
#include <domain_logic/sign_transceiver.hpp>


void main_task(void *);


enum class main_task_states {
	CHOOSE_TASK,

	START_SETUP_SERVER,
	WAIT_FOR_SETUP_COMPLETE,
	STOP_SETUP_SERVER,
	CONFIG_ERROR,

	CONNECT_TO_WIFI,
	PREPARE_CONNECTION,
	CONNECT_TO_PLUGIN,
	VALIDATE_CONNECTION,
	RECEIVE_MESSAGE,
	SETUP_ERROR
};

constexpr const char* main_task_state_name(main_task_states state) {
	constexpr std::array<const char*, 11> names {
		"choose_task",
		"start_setup_server",
		"wait_for_setup_complete",
		"stop_setup_server",
		"config_error",
		"connect_to_wifi",
		"prepare_connection",
		"connect_to_plugin",
		"validate_connection",
		"receive_message",
		"setup_error"
	};
	using enum_t = std::underlying_type_t<main_task_states>;
	const auto index = static_cast<enum_t>(state);
	if (index >= 0 and index < names.size()) {
		return names[index];
	} else {
		return "(unknown state)";
	}
}


struct choose_task {
	static constexpr auto states = std::array{
		main_task_states::CHOOSE_TASK,
		main_task_states::SETUP_ERROR,
		main_task_states::CONFIG_ERROR
	};
	static main_task_states run(
		main_task_states state,
		std::error_code &error
	);
};


//------------[ setup task ]------------//

struct start_setup_server {
	static constexpr auto states = std::array{
		main_task_states::START_SETUP_SERVER
	};
	static main_task_states run(
		main_task_states,
		std::error_code &error,
		wifi::access_point_handler &wifi_ap,
		int &num_retries
	);
};

struct wait_for_setup_complete {
	static constexpr auto states = std::array{
		main_task_states::WAIT_FOR_SETUP_COMPLETE
	};
	static main_task_states run(
		main_task_states,
		std::error_code &error,
		wifi::access_point_handler &wifi_ap
	);
};

struct stop_setup_server {
	static constexpr auto states = std::array{
		main_task_states::STOP_SETUP_SERVER
	};
	static main_task_states run(
		main_task_states,
		std::error_code &error,
		wifi::access_point_handler &wifi_ap
	);
};


//------------[ sign task ]------------//

struct connect_to_wifi {
	static constexpr auto states = std::array{
		main_task_states::CONNECT_TO_WIFI
	};
	static main_task_states run(
		main_task_states,
		std::error_code &error,
		wifi::client_handler& wifi_client,
		int& num_retries
	);
};

struct prepare_connection {
	static constexpr auto states = std::array{
		main_task_states::PREPARE_CONNECTION
	};
	static main_task_states run(
		main_task_states,
		std::error_code& error,
		wifi::client_handler& wifi_client,
		lwip_socket_acceptor& acceptor,
		hmac_sha_512_engine& sha_engine,
		aes_256_engine& aes_engine
	);
};

struct connect_to_plugin {
	static constexpr auto states = std::array{
		main_task_states::CONNECT_TO_PLUGIN
	};
	static main_task_states run(
		main_task_states,
		std::error_code& error,
		wifi::client_handler& wifi_client,
		lwip_socket_acceptor& acceptor,
		hmac_sha_512_engine& sha_engine,
		aes_256_engine& aes_engine,
		lwip_socket_connection& conn
	);
};

struct validate_connection {
	static constexpr auto states = std::array{
		main_task_states::VALIDATE_CONNECTION
	};
	enum class internal_state {
		CREATE_CHALLENGE,
		SEND_CHALLENGE,
		RECEIVE_ANSWER,
		SEND_OK,
		RECEIVE_CHALLENGE,
		SEND_ANSWER,
		RECEIVE_OK
	};
	static main_task_states run(
		main_task_states,
		std::error_code& error,
		wifi::client_handler& wifi_client,
		lwip_socket_acceptor& acceptor,
		hmac_sha_512_engine& sha_engine,
		aes_256_engine& aes_engine,
		lwip_socket_connection& conn,
		internal_state& state,
		std::array<u8, 64>& buffer,
		std::array<u8, 64>& hash,
		std::span<u8>& io_bytes
	);
};

struct reveive_message {
	static constexpr auto states = std::array{
		main_task_states::RECEIVE_MESSAGE
	};
	enum class internal_state {
		INIT_RECEIVE,
		RECEIVE_HEADER,
		RECEIVE_BODY,
		HANDLE_MESSAGE
	};
	static main_task_states run(
		main_task_states,
		std::error_code& error,
		wifi::client_handler& wifi_client,
		lwip_socket_acceptor& acceptor,
		hmac_sha_512_engine& sha_engine,
		aes_256_engine& aes_engine,
		lwip_socket_connection& conn,
		internal_state& state,
		sign_transceiver& transceiver,
		sign_header& header,
		sign_message& message,
		std::span<u8>& io_bytes
	);
};
