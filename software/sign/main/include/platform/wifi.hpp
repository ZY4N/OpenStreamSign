#pragma once

#include <string>
#include <util/uix.hpp>

namespace wifi {

	using namespace ztu::uix;

	u32 parseIP(const char *ip);

	namespace client {

		int connect(
			const std::string_view& ssid, const std::string_view& password,
			u32 ip, u32 netmask, u32 gateway,
			int numRetries
		);

		void disconnect();

	}

	namespace ap {

		bool create(
			const std::string_view& ssid,
			const std::string_view& password,
			u8 channel, u8 maxConnections
		);

		void destroy();
		
	}
}
