#include <platform/lwip_socket_acceptor.hpp>

#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>


constexpr auto TAG = "SOCKET_ACCEPTOR";


static inline std::error_code make_system_error(int code) {
	using errc_t = std::underlying_type_t<std::errc>;
	const auto errc = static_cast<std::errc>(static_cast<errc_t>(code));
	return std::make_error_code(errc);
}


std::error_code lwip_socket_acceptor::initialize(const uint16_t port) {

	int ip_protocol = 0;
	
	struct sockaddr_storage dest_addr;

	struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
	dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
	dest_addr_ip4->sin_family = AF_INET;
	dest_addr_ip4->sin_port = htons(port);
	ip_protocol = IPPROTO_IP;


	lwip_safe_fd listen_sock{ ::socket(AF_INET, SOCK_STREAM, ip_protocol) };

	if (listen_sock.fd < 0) {
		return make_system_error(errno);
	}

	int opt = 1;
	if (setsockopt(listen_sock.fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) != 0) {
		return make_system_error(errno);
	}

	if (bind(listen_sock.fd, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) != 0) {
		return make_system_error(errno);
	}

	if (::listen(listen_sock.fd, 1) != 0) {
		return make_system_error(errno);
	}

	m_socket = std::move(listen_sock);

	return make_system_error(0);
}

std::error_code lwip_socket_acceptor::listen(lwip_socket_connection &connection) {

	struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
	socklen_t addr_len = sizeof(source_addr);

	lwip_safe_fd conn{ accept(m_socket.fd, (struct sockaddr *)&source_addr, &addr_len) };
	if (conn.fd < 0) {
		return std::error_code(errno, std::system_category());
	}
		
	connection.socket() = std::move(conn);

	return make_system_error(0);
}
