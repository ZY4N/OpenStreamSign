#pragma once

#include "http_handlers.hpp"

namespace config_webserver {

	inline httpd_handle_t server = nullptr;

	inline esp_err_t start() {

		httpd_ssl_config_t conf = HTTPD_SSL_CONFIG_DEFAULT();
		conf.httpd.max_uri_handlers = 16;

		extern const unsigned char servercert_start[] asm("_binary_servercert_pem_start");
		extern const unsigned char servercert_end[]   asm("_binary_servercert_pem_end");
		conf.servercert = servercert_start;
		conf.servercert_len = servercert_end - servercert_start;

		extern const unsigned char prvtkey_pem_start[] asm("_binary_prvtkey_pem_start");
		extern const unsigned char prvtkey_pem_end[]   asm("_binary_prvtkey_pem_end");
		conf.prvtkey_pem = prvtkey_pem_start;
		conf.prvtkey_len = prvtkey_pem_end - prvtkey_pem_start;
		
		if (auto ret = httpd_ssl_start(&server, &conf); ESP_OK != ret) {
			return ret;
		}

		for (const auto &handler : handlers) {
			if (auto ret = httpd_register_uri_handler(server, &handler); ret != ESP_OK) {
				return ret;
			}
		}

		return ESP_OK;
	}

	inline esp_err_t stop() {
		return httpd_ssl_stop(server);
	}
}
