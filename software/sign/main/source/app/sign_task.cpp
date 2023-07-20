#include <app/sign_task.hpp>

#include <domain_logic/sign.hpp>

#include <hmac_sha_512_handshake.hpp>
#include <domain_logic/sign_transceiver.hpp>

#include <platform/lwip_socket_acceptor.hpp>
#include <platform/wifi.hpp>

#include <esp_log.h>


constexpr auto SIGN_TAG = "SIGN_TASK";

static inline void log_error_code(const char *origin, const std::error_code &e) {
	ESP_LOGE(SIGN_TAG, "[%s][%s]: %s", origin, e.category().name(), e.message().c_str());
}

// variant index != enum index Alarm
void handleCommand(const sign_message &msg) {
	switch (msg.type()) {
		using enum sign_message_type;
		case CHANGE_STATE: {
			const auto &[ state ] = msg.get<CHANGE_STATE>();
			ESP_LOGI(SIGN_TAG, "Entering mode: %d", (int) state);
			sign.animation_controller.setState(state);
			break;
		};
		case SET_ANIMATION: {
			const auto &[ state, animation ] = msg.get<SET_ANIMATION>();
			sign.animation_controller.setAnimation(state, animation);
			break;
		};
		case HEARTBEAT: {
			ESP_LOGI(SIGN_TAG, "got heartbeat");
			break;
		};
    default: {
			ESP_LOGI(SIGN_TAG, "unimplemented command");
		}
	}
}

void sign_task() {

	ESP_LOGI(SIGN_TAG, "Starting sign task");

	sign.animation_controller.setState(sign_state::PROCESSING);

	bool connectedToWifi = false;

	{
		ESP_LOGI(SIGN_TAG, "SSID");
		const auto ssid = sign.storage.get<storage_keys::SSID>();

		ESP_LOGI(SIGN_TAG, "password");
		const auto password = sign.storage.get<storage_keys::PASSWORD>();

		ESP_LOGI(SIGN_TAG, "ip");
		const auto ip = sign.storage.get<storage_keys::IP_ADDRESS>();

		ESP_LOGI(SIGN_TAG, "subnetmask");
		const auto subnetmask = sign.storage.get<storage_keys::SUBNETMASK>();

		ESP_LOGI(SIGN_TAG, "gateway");
		const auto gateway = sign.storage.get<storage_keys::GATEWAY>();

		ESP_LOGI(SIGN_TAG, "Connecting: SSID: '%s' PW: '%s' IP: '%lu' MASK: '%lu' GATEWAY: '%lu'", ssid.c_str(), password.c_str(), ip, subnetmask, gateway);

		int error = wifi::client::connect(
			ssid.sv(),
			password.sv(),
			ip, subnetmask, gateway, 10
		);

		if (error > 70) {
			ESP_LOGI(SIGN_TAG, "Wifi connection success");
			connectedToWifi = true;
		} else {
			ESP_LOGE(SIGN_TAG, "Wifi connection  %d", error);
			goto endTask;
		}
		/*
		if (not wifi::connect(
			{ ssid.begin(), ssid.end() },
			{ password.begin(), password.end() },
			ip, subnetmask, gateway, 10
		)) {
			ESP_LOGE(SIGN_TAG, "Wifi connection failed");
			goto endTask;
		}*/


		const auto secret = sign.storage.get<storage_keys::SECRET>();

		const auto sha512_secret = std::span{ secret.begin(), 512 / 8 };
		const auto aes256_secret = std::span{ sha512_secret.end(), 256 / 8 };

		std::error_code e;

		hmac_sha_512_engine sha_engine;
		if ((e = sha_engine.init(sha512_secret)))
			goto endTask;

		aes_256_engine aes_engine;
		if ((e = aes_engine.init(aes256_secret)))
			goto endTask;

		lwip_socket_connection conn;
		hmac_sha_512_handshake validator{ sha_engine, conn };
		sign_transceiver transceiver{ aes_engine, conn };
		
		const auto port = sign.storage.get<storage_keys::PORT>();

		lwip_socket_acceptor acceptor;
		if ((e = acceptor.initialize(port))) {
			log_error_code("SOCKET_INIT_ERROR", e);
			goto endTask;
		}
		ESP_LOGI(SIGN_TAG, "socket acceptor successfuly initialized");


		while (true) {
			ESP_LOGI(SIGN_TAG, "Listening on port %d", port);

			if ((e = acceptor.listen(conn))) {
				log_error_code("SOCKET_LSITEN_ERROR", e);
				continue;
			}

			if ((e = conn.setReceiveTimeoutInterval(CONFIG_RECEIVE_TIMEOUT, 0))) {
				log_error_code("SOCKET_SETTING_TIMEOUT_ERROR", e);
				continue;
			}


			ESP_LOGI(SIGN_TAG, "Validating connection");
			if ((e = validator.validate(true))) {
				log_error_code("CHALLENGE_ERROR", e);
				continue;
			}
			ESP_LOGI(SIGN_TAG, "Handshake complete");

			sign.animation_controller.setState(sign_state::CONNECTED);

			sign_message msg;

			while (true) {
				if ((e = transceiver.receive(msg))) {
					if (
						e.category() == std::system_category() and // probably networking issue
						e.value() != EAGAIN // timeout is not a permanent error
					) {
						log_error_code("RECEIVE_ERROR", e);
						break;
					}
				}

				handleCommand(msg);

				if (sign.switch_task.test()) {
					sign.switch_task.clear();
					goto endTask;
				}
			}
			sign.animation_controller.setState(sign_state::IDLE);
		}
	}

endTask:
	if (connectedToWifi) {
		ESP_LOGI(SIGN_TAG, "Trying to disconnect from wifi");
		wifi::client::disconnect();
	}

	sign.animation_controller.setState(sign_state::IDLE);
}
