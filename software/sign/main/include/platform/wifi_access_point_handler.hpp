#pragma once

#include "wifi_generic_handler.hpp"
#include <string_view>
#include <platform/esp_error.hpp>


namespace wifi {
	class access_point_handler : public generic_handler {
	public:
		access_point_handler() = default;

		access_point_handler(access_point_handler&& other)
			: generic_handler{ std::move(other) } {}

		using generic_handler::operator=;

		[[nodiscard]] std::error_code create(
			const std::string_view& ssid,
			const std::string_view& password,
			u8 channel, u8 maxConnections,
			u32 timeout_ms = 0
		);

		void destroy();

		~access_point_handler();

	private:
		static void event_handler(
			void *arg,
			esp_event_base_t event_base,
			i32 event_id,
			void* event_data
		);
		
		inline static constexpr auto TAG = "WIFI_ACCESS_POINT";
	};
}
