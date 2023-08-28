#include <platform/wifi_access_point_handler.hpp>
#include <platform/esp_error.hpp>
#include <freertos/task.h>
#include <atomic>
#include <esp_log.h>

std::error_code wifi::access_point_handler::create(
	const std::string_view& ssid,
	const std::string_view& password,
	ztu::u8 channel, ztu::u8 maxConnections,
	ztu::u32 timeout_ms
) {
	using enum esp_error::codes;
	using esp_error::make_error_code;
	
	esp_err_t code;
	std::atomic_flag connected = ATOMIC_FLAG_INIT;
	esp_event_handler_instance_t instance_got_ip{ nullptr }, instance_any_id{ nullptr };
	
	{
		if ((code = generic_init()) != ESP_OK)
			goto on_error;

		if ((m_handle = esp_netif_create_default_wifi_ap()) == nullptr) {
			code = static_cast<int>(FAIL);
			goto on_error;
		}
	
		const wifi_init_config_t default_wifi_config = WIFI_INIT_CONFIG_DEFAULT();
		if ((code = esp_wifi_init(&default_wifi_config)) != ESP_OK)
			goto on_error;

		
		if ((code = esp_event_handler_instance_register(
			WIFI_EVENT, ESP_EVENT_ANY_ID, event_handler, &connected, &instance_any_id
		)) != ESP_OK) goto on_error;

		
		if ((code = esp_event_handler_instance_register(
			IP_EVENT, IP_EVENT_STA_GOT_IP, event_handler, &connected, &instance_got_ip
		)) != ESP_OK) goto on_error;

		auto wifi_config = wifi_config_t{
			.ap = {
				.ssid_len = static_cast<uint8_t>(ssid.length()),
				.channel = channel,
				.authmode = WIFI_AUTH_WPA_WPA2_PSK,
				.max_connection = maxConnections,
				.pmf_cfg = { .required = false }
			}
		};

		if (not copyStringToBuffer(ssid, wifi_config.ap.ssid)) {
			code = static_cast<int>(ERR_WIFI_SSID);
			goto on_error;
		}
		
		if (not copyStringToBuffer(password, wifi_config.ap.password)) {
			code = static_cast<int>(ERR_WIFI_PASSWORD);
			goto on_error;
		}

		if (ssid.empty()) {
			wifi_config.ap.authmode = WIFI_AUTH_OPEN;
		}

		if ((code = esp_wifi_set_mode(WIFI_MODE_AP)) != ESP_OK)
			goto on_error;

		if ((code = esp_wifi_set_config(WIFI_IF_AP, &wifi_config)) != ESP_OK)
			goto on_error;

		if ((code = esp_wifi_start()) != ESP_OK)
			goto on_error;

		// Wait for fixed interval or until connected
		if (timeout_ms) {
			vTaskDelay(timeout_ms / portTICK_PERIOD_MS);
		} else {
			connected.wait(false);
		}

		if ((code = esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip)) != ESP_OK)
			goto on_error;

		if ((code = esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id)) != ESP_OK)
			goto on_error;
	}

	if (connected.test())
		return make_error_code(OK); 

	code = static_cast<int>(ERR_WIFI_NOT_CONNECT);

on_error:
	if (m_handle) {
		esp_netif_destroy_default_wifi(m_handle);
		m_handle = nullptr;
	}

	esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip);
	esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id);

	return make_error_code(static_cast<esp_error::codes>(code));
}

void wifi::access_point_handler::destroy() {
	generic_deinit();
}

wifi::access_point_handler::~access_point_handler() {
	destroy();
}

void wifi::access_point_handler::event_handler(
	void *args,
	esp_event_base_t event_base,
	ztu::i32 event_id,
	void* event_data
) {
	ESP_LOGI(TAG, "event_base: %d event_id: %d", int(event_base), int(event_id));

	auto &connected = *reinterpret_cast<std::atomic_flag*>(args);
	if (event_base == WIFI_EVENT) {
		switch (event_id) {
			case WIFI_EVENT_AP_START:
				connected.test_and_set();
				connected.notify_one();
				esp_wifi_connect();
				ESP_LOGW(TAG, "Wifi AP up");
				break;
			case WIFI_EVENT_AP_STOP:
				ESP_LOGW(TAG, "Wifi AP down");
				break;
			case WIFI_EVENT_AP_STACONNECTED: {
				const auto* event = (wifi_event_ap_staconnected_t*) event_data;
       			ESP_LOGI(TAG, "station connected: aid=%d mac=", event->aid);
				ESP_LOG_BUFFER_HEX(TAG, event->mac, sizeof(event->mac));
				break;
			}
			case WIFI_EVENT_AP_STADISCONNECTED: {
				const auto* event = (wifi_event_ap_stadisconnected_t*) event_data;
       			ESP_LOGI(TAG, "station connected: aid=%d mac=", event->aid);
				ESP_LOG_BUFFER_HEX(TAG, event->mac, sizeof(event->mac));
				break;
			}
		}
	}
}