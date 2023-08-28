#pragma once

#include <util/uix.hpp>
#include <string_view>
#include "esp_wifi.h"

namespace wifi {

	using namespace ztu::uix;

	u32 parseIP(const char *ip);

	class generic_handler {
	protected:
		static inline int next_id = 0;
		template<size_t Size>
		static bool copyStringToBuffer(const std::string_view& str, uint8_t (&buffer)[Size]);

	public:
		generic_handler();// = default;

		generic_handler(const generic_handler&) = delete;
		generic_handler& operator=(const generic_handler& other) = delete;

		generic_handler(generic_handler&& other);
		generic_handler& operator=(generic_handler&& other);

		~generic_handler();

	protected:
		[[nodiscard]] int generic_init();
		void generic_deinit();

	protected:
		esp_netif_t *m_handle{ nullptr };
		int id{ 0 };
	};
}


template<size_t Size>
bool wifi::generic_handler::copyStringToBuffer(const std::string_view& str, uint8_t (&buffer)[Size]) {
	if (str.size() >= Size)
		return false;

	std::copy(str.begin(), str.end(), buffer);
	buffer[str.size()] = '\0';
	
	return true;
};
