#pragma once

#include "wifi_generic_handler.hpp"
#include <string_view>
#include <platform/esp_error.hpp>
#include "freertos/event_groups.h"
#include <atomic>

namespace wifi {
	class client_handler : public generic_handler {
	public:
		client_handler() = default;

		client_handler(client_handler&& other)
			: generic_handler{ std::move(other) } {}

		using generic_handler::operator=;

		[[nodiscard]] std::error_code connect(
			const std::string_view& ssid, const std::string_view& password,
			u32 ip, u32 netmask, u32 gateway,
			u32 num_retries, u32 timeout_ms = 0
		);

		void disconnect();

		~client_handler();

	private:
		struct connection_state {
			u32 ip, netmask, gateway;
			u32 current_retries, max_retries;
			esp_netif_t *handle{ nullptr };
			std::atomic_flag &connected;

			void event_handler(
				esp_event_base_t event_base,
				i32 event_id,
				void* event_data
			);
		};

		inline static constexpr auto TAG = "WIFI_CLIENT";
	};
}
