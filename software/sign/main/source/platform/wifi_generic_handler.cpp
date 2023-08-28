#include <platform/wifi_generic_handler.hpp>
#include <netdb.h>


static constexpr auto TAG = "wifi_generic_handler";

ztu::u32 wifi::parseIP(const char *ip) {
	return ipaddr_addr(ip);
}

wifi::generic_handler::generic_handler() {
	id = next_id++;
}

wifi::generic_handler::generic_handler(generic_handler&& other) {
	id = next_id++;
	m_handle = other.m_handle;
	other.m_handle = nullptr;
}

wifi::generic_handler& wifi::generic_handler::operator=(generic_handler&& other) {
	if (&other != this) {
		generic_deinit();
		m_handle = other.m_handle;
		other.m_handle = nullptr;
	}
	return *this;
}

esp_err_t wifi::generic_handler::generic_init() {
	static bool tcp_ip_initialized = false;
	static bool event_loop_initialized = false;

	if (not tcp_ip_initialized) {
		const auto errc = esp_netif_init();
		if (errc != ESP_OK) {			
			return errc;
		}
		tcp_ip_initialized = true;
	}

	if (not event_loop_initialized) {
		const auto errc = esp_event_loop_create_default();
		if (errc != ESP_OK and errc != ESP_ERR_INVALID_STATE) {
			return errc;
		}
		event_loop_initialized = true;
	}

	return ESP_OK;
}

void wifi::generic_handler::generic_deinit() {
	if (m_handle) {
		esp_netif_destroy_default_wifi(m_handle);
		esp_wifi_stop();
		m_handle = nullptr;
	}
}

wifi::generic_handler::~generic_handler() {
	generic_deinit();
}
