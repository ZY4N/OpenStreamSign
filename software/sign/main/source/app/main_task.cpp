#include <app/main_task.hpp>

#include <platform/basic_button.hpp>
#include <util/state_machine.hpp>
#include <domain_logic/sign.hpp>
#include <esp_log.h>


constexpr auto MAIN_TAG = "MAIN_TASK";

void main_task(void *) {

	if (not sign.storage.open("storage")) {
		ESP_LOGE(MAIN_TAG, "[NVS_INIT_ERROR]: nvs init failed");
		return;
	}

	/*ESP_LOGI(TAG, "SETTING FLAG");
	sign.storage.set<storage_keys::SETUP_DONE>(false);
	ESP_LOGI(TAG, "FLAG SET");*/

	sign.animation_controller.init(sign_state::IDLE);

	sign.switch_task.clear();

	uint32_t counter = 0;

	const auto setup_button = basic_button(
		static_cast<gpio_num_t>(CONFIG_RESET_BUTTON_PIN),
		false,
		[&counter](basic_button_event e) {
			if (e == basic_button_event::DOWN) {
				ESP_DRAM_LOGI("BUTTON", "set");
				sign.switch_task.test_and_set();
				sign.switch_task.notify_all();
			}
		}
	);


	ESP_LOGI(MAIN_TAG, "Starting sign task");

	auto on_state_change = [](main_task_states from, main_task_states& to) {
		using enum main_task_states;

		// Allow idle task to run.
		vTaskDelay(1);

		// Check if setup button has been pressed.
		if (sign.switch_task.test()) {
			ESP_DRAM_LOGI(MAIN_TAG, "Button press detected");
			sign.switch_task.clear();
			sign.storage.set<storage_keys::SETUP_DONE>(
				not sign.storage.get<storage_keys::SETUP_DONE>()
			);
			to = CHOOSE_TASK;
		}

		switch (to) {
			case START_SETUP_SERVER:
			case STOP_SETUP_SERVER:
			case CONNECT_TO_WIFI:
			case PREPARE_CONNECTION:
				sign.animation_controller.setState(sign_state::PROCESSING);
				break;
			case WAIT_FOR_SETUP_COMPLETE:
				sign.animation_controller.setState(sign_state::SETUP);
				break;
			case CONNECT_TO_PLUGIN:
				sign.animation_controller.setState(sign_state::CONNECTED); // technically wrong...
				break;
			case CHOOSE_TASK:
			case SETUP_ERROR:
			case CONFIG_ERROR:
				sign.animation_controller.setState(sign_state::IDLE);
				break;
			default:
				break;
		}

		if (from != to) {
			ESP_LOGI(MAIN_TAG, "%s -> %s", main_task_state_name(from), main_task_state_name(to));
		}

		return from != CONFIG_ERROR;
	};

	state_machine<
	    decltype(on_state_change),
		choose_task,
		start_setup_server,
		wait_for_setup_complete,
		stop_setup_server,
		connect_to_wifi,
		prepare_connection,
		validate_connection,
		connect_to_plugin,
		reveive_message
	> task(std::move(on_state_change));

	task.run(main_task_states::CHOOSE_TASK);

	vTaskDelete(nullptr);
}


static inline void handleCommand(const sign_message &msg) {
	static constexpr auto TAG = "HANDLE_COMMAND";
	switch (msg.type()) {
		using enum sign_message_type;
		case CHANGE_STATE: {
			ESP_LOGI(TAG, "change state");
			const auto &[ state ] = msg.get<CHANGE_STATE>();
			ESP_LOGI(TAG, "Entering state: %d", (int) state);
			sign.animation_controller.setState(state);
			break;
		};
		case SET_ANIMATION: {
			ESP_LOGI(TAG, "set animation");
			const auto &[ state, animation ] = msg.get<SET_ANIMATION>();
			sign.animation_controller.setAnimation(state, animation);
			break;
		};
		case HEARTBEAT: {
			ESP_LOGI(TAG, "heartbeat");
			break;
		};
    default: {
			ESP_LOGI(TAG, "(unimplemented command)");
		}
	}
}


static inline void log_error_code(const char *origin, const std::error_code &e) {
	ESP_LOGE(origin, "[%s]: %s", e.category().name(), e.message().c_str());
}


main_task_states choose_task::run(main_task_states state) {
	using enum main_task_states;

	static constexpr auto TAG = main_task_state_name(CHOOSE_TASK);

	if (state == SETUP_ERROR) {
		ESP_LOGE(TAG, "Detected error in user setup. Please reconfigure via the webinterface.");
		sign.storage.set<storage_keys::SETUP_DONE>(false);
	} else if (state == CONFIG_ERROR) {
		ESP_LOGE(TAG, "Detected error in base config.");
		return CONFIG_ERROR;
	} 

	if (sign.storage.get<storage_keys::SETUP_DONE>()) {
		return CONNECT_TO_WIFI;
	} else {
		return START_SETUP_SERVER;
	}
}

main_task_states start_setup_server::run(
	main_task_states,
	wifi::access_point_handler& wifi_ap,
	std::error_code& error,
	int& num_retries
) {
	using enum main_task_states;

	static constexpr auto TAG = main_task_state_name(START_SETUP_SERVER);


	ESP_LOGI(TAG, "timeout: %d", CONFIG_STATE_TIMEOUT_MS);


	error = wifi_ap.create(
		CONFIG_AP_SSID, CONFIG_AP_PASSWORD,
		1, 2, CONFIG_STATE_TIMEOUT_MS
	);

	if (error) {
		log_error_code(TAG, error);

		num_retries++;
		switch (static_cast<esp_error::codes>(error.value())) {
			using enum esp_error::codes;
			case ERR_WIFI_SSID:
			case ERR_WIFI_PASSWORD:
				return CONFIG_ERROR;
			default:
				return num_retries > 5 ? CONFIG_ERROR : START_SETUP_SERVER;
		}
	}

	config_webserver::start();

	return WAIT_FOR_SETUP_COMPLETE;
}

main_task_states wait_for_setup_complete::run(
	main_task_states,
	wifi::access_point_handler& wifi_ap
) {
	using enum main_task_states;

	static constexpr auto TAG = main_task_state_name(WAIT_FOR_SETUP_COMPLETE);

	vTaskDelay(CONFIG_STATE_TIMEOUT_MS / portTICK_PERIOD_MS);

	if (sign.switch_task.test()) {
		sign.switch_task.clear();
		return STOP_SETUP_SERVER;
	}
	
	return WAIT_FOR_SETUP_COMPLETE;
}

main_task_states stop_setup_server::run(
	main_task_states,
	wifi::access_point_handler& wifi_ap
) {
	using enum main_task_states;

	static constexpr auto TAG = main_task_state_name(STOP_SETUP_SERVER);

	ESP_LOGI(TAG, "Stopping server");
	config_webserver::stop();

	ESP_LOGI(TAG, "Stopping wifi AP");
	wifi_ap.destroy();

	ESP_LOGI(TAG, "Saving");
	// set done byte and save config data to flash memory
	sign.storage.set<storage_keys::SETUP_DONE>(true);
	sign.storage.save();
	
	return CHOOSE_TASK;
}


//------------[ sign task ]------------//

main_task_states connect_to_wifi::run(
	main_task_states,
	std::error_code& error,
	wifi::client_handler& wifi_client,
	int& num_retries
) {
	using enum main_task_states;

	static constexpr auto TAG = main_task_state_name(CONNECT_TO_WIFI);

	const auto ssid			= sign.storage.get<storage_keys::SSID>();
	const auto password		= sign.storage.get<storage_keys::PASSWORD>();
	const auto ip			= sign.storage.get<storage_keys::IP_ADDRESS>();
	const auto subnetmask	= sign.storage.get<storage_keys::SUBNETMASK>();
	const auto gateway		= sign.storage.get<storage_keys::GATEWAY>();

	ESP_LOGI(TAG, "Connecting: SSID: '%s' PW: '%s' IP: '%lu' MASK: '%lu' GATEWAY: '%lu'", ssid.c_str(), password.c_str(), ip, subnetmask, gateway);
	
	error = wifi_client.connect(
		ssid.sv(), password.sv(),
		ip, subnetmask, gateway,
		CONFIG_STATE_TIMEOUT_MS
	);

	if (error) {
		++num_retries;
		log_error_code(TAG, error);
		switch (static_cast<esp_error::codes>(error.value())) {
			using enum esp_error::codes;
			case ERR_WIFI_SSID:
			case ERR_WIFI_PASSWORD:
			default:
				return num_retries > 5 ? SETUP_ERROR : CONNECT_TO_WIFI;
		}
	}

	return PREPARE_CONNECTION;
}

main_task_states prepare_connection::run(
	main_task_states,
	std::error_code& error,
	wifi::client_handler& wifi_client,
	lwip_socket_acceptor& acceptor,
	hmac_sha_512_engine& sha_engine,
	aes_256_engine& aes_engine
) {
	using enum main_task_states;

	static constexpr auto TAG = main_task_state_name(PREPARE_CONNECTION);

	const auto port = sign.storage.get<storage_keys::PORT>();
	ESP_LOGI(TAG, "port: %d", port);
	if ((error = acceptor.initialize(port))) {
		log_error_code("SOCKET_INIT_ERROR", error);
		return CONNECT_TO_WIFI;
	}

	ESP_LOGI(TAG, "socket acceptor successfuly initialized");


	const auto secret = sign.storage.get<storage_keys::SECRET>();

	const auto sha512_secret = std::span{ secret.begin(), 512 / 8 };
	const auto aes256_secret = std::span{ sha512_secret.end(), 256 / 8 };		

	if ((error = sha_engine.init(sha512_secret))) {
		log_error_code(TAG, error);
		return SETUP_ERROR;
	}

	if ((error = aes_engine.init(aes256_secret))) {
		log_error_code(TAG, error);
		return SETUP_ERROR;
	}

	return CONNECT_TO_PLUGIN;
}

main_task_states connect_to_plugin::run(
	main_task_states,
	std::error_code& error,
	wifi::client_handler& wifi_client,
	lwip_socket_acceptor& acceptor,
	hmac_sha_512_engine& sha_engine,
	aes_256_engine& aes_engine,
	lwip_socket_connection& conn
) {
	using enum main_task_states;

	static constexpr auto TAG = main_task_state_name(CONNECT_TO_PLUGIN);

	if ((error = acceptor.listen(conn, CONFIG_STATE_TIMEOUT_MS))) {
		if (error.value() != EAGAIN) {
			log_error_code(TAG, error);
		}
		return CONNECT_TO_PLUGIN;
	}

	ESP_LOGI(MAIN_TAG, "Client connected");

	if ((error = conn.set_send_timeout(CONFIG_STATE_TIMEOUT_MS))) {
		log_error_code(TAG, error);
		return SETUP_ERROR;
	}

	if ((error = conn.set_receive_timeout(CONFIG_STATE_TIMEOUT_MS))) {
		log_error_code(TAG, error);
		return SETUP_ERROR;
	}

	return VALIDATE_CONNECTION;
}

main_task_states validate_connection::run(
	main_task_states,
	std::error_code& error,
	wifi::client_handler& wifi_client,
	lwip_socket_acceptor& acceptor,
	hmac_sha_512_engine& sha_engine,
	aes_256_engine& aes_engine,
	lwip_socket_connection& conn,
	validate_connection::internal_state &state,
	std::array<u8, 64>& buffer,
	std::array<u8, 64>& hash,
	std::span<u8>& io_bytes
) {
	static constexpr auto TAG = main_task_state_name(main_task_states::VALIDATE_CONNECTION);

	switch (state) {
		using enum internal_state;
		case CREATE_CHALLENGE: {
			fill_random(buffer);
			if ((error = sha_engine.hash(buffer, hash))) goto on_error;
			io_bytes = buffer;
			state = SEND_CHALLENGE;
			break;
		}
		case SEND_CHALLENGE:
		case SEND_ANSWER:
		case SEND_OK: {
			auto &o_bytes = *reinterpret_cast<std::span<const u8>*>(&io_bytes);
			if ((error = conn.send(o_bytes))) goto on_error;
			if (io_bytes.empty()) {
				switch (state) {
					case SEND_CHALLENGE: {
						io_bytes = buffer;
						state = RECEIVE_ANSWER;
						break;
					}
					case SEND_OK: {
						if (buffer[0]) {
							io_bytes = buffer;
							state = RECEIVE_CHALLENGE;
						} else {
							ESP_LOGE(TAG, "Peer was not able to solve buffer.");
							return main_task_states::SETUP_ERROR;
						}
						break;
					}
					case SEND_ANSWER: {
						io_bytes = { buffer.data(), 1 };
						state = RECEIVE_OK;
						break;
					}
					default: break;
				}
			}
			break;
		}
		case RECEIVE_ANSWER:
		case RECEIVE_CHALLENGE:
		case RECEIVE_OK: {
			if ((error = conn.receive(io_bytes))) goto on_error;
			if (io_bytes.empty()) {
				switch (state) {
					case RECEIVE_ANSWER: {
						buffer[0] = { hash == buffer };
						io_bytes = { buffer.data(), 1 };
						state = SEND_OK;
						break;
					}
					case RECEIVE_CHALLENGE: {
						if ((error = sha_engine.hash(buffer, hash))) goto on_error;
						io_bytes = hash;
						state = SEND_ANSWER;
						break;
					}
					case RECEIVE_OK: {
						if (buffer[0]) {
							ESP_LOGI(TAG, "Connection validated.");
							return main_task_states::RECEIVE_MESSAGE;
						} else {
							ESP_LOGE(TAG, "Could not olve buffer sent by peer");
							return main_task_states::SETUP_ERROR;
						}
					}
					default: break;
				}
			}
			break;
		}
	}

	return main_task_states::VALIDATE_CONNECTION;

on_error:
	switch (error.value()) {
		case EAGAIN: // just a timeout
			return main_task_states::VALIDATE_CONNECTION;
		case ENOTCONN:
		case ECONNRESET:
		case EPIPE:
		case EBADF:
		case EDESTADDRREQ:
		case ENOTSOCK: // connection down
			log_error_code(TAG, error);
			return main_task_states::CONNECT_TO_PLUGIN;
		default: // something else
			log_error_code(TAG, error);
			return main_task_states::SETUP_ERROR;
	}
}

main_task_states reveive_message::run(
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
) {
	using enum main_task_states;

	static constexpr auto TAG = main_task_state_name(RECEIVE_MESSAGE);

	transceiver.engine() = &aes_engine;

	switch (state) {
		using enum internal_state;
		case INIT_RECEIVE: {
			io_bytes = transceiver.header_packet_buffer();
			state = RECEIVE_HEADER;
			break;
		}
		case RECEIVE_HEADER:
		case RECEIVE_BODY: {
			if ((error = conn.receive(io_bytes))) goto on_error;
			if (io_bytes.empty()) {
				if (state == RECEIVE_HEADER) {
					if ((error = transceiver.decrypt_header(header, io_bytes)))
						goto on_error;
					state = RECEIVE_BODY;
				} else {
					if ((error = transceiver.decrypt_body(header, message)))
						goto on_error;
					state = HANDLE_MESSAGE;
				}
			}
			break;
		}
		case HANDLE_MESSAGE: {
			handleCommand(message);
			state = INIT_RECEIVE;
			break;
		}
	}
	
	return RECEIVE_MESSAGE;

on_error:
	if (error.category() == std::generic_category()) {
		switch (error.value()) {
			case EAGAIN:
				return RECEIVE_MESSAGE;
			case ENOTCONN:
			case ECONNRESET:
			case EPIPE:
			case EBADF:
			case EDESTADDRREQ:
			case ENOTSOCK:
				log_error_code(TAG, error);
				return CONNECT_TO_PLUGIN;
			default:
				log_error_code(TAG, error);
				return SETUP_ERROR;
		}
	} else if (error.category() == aes_transceiver_category()) {
		log_error_code(TAG, error);
		switch (error.value()) {
			using enum aes_transceiver_error::codes;
			case static_cast<int>(SERIALIZATION_ERROR):
			case static_cast<int>(DESERIALIZATION_ERROR):
				state = internal_state::INIT_RECEIVE;
				return RECEIVE_MESSAGE;
			case static_cast<int>(INVALID_MESSAGE_TYPE):
			case static_cast<int>(INVALID_MESSAGE_SIZE):
				return CONNECT_TO_PLUGIN;
			default:
				return SETUP_ERROR;
		}
	} else {
		log_error_code(TAG, error);
		return SETUP_ERROR;
	}
}

