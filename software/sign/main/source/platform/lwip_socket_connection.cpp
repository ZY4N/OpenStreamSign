#include <platform/lwip_socket_connection.hpp>

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>


static inline std::error_code make_system_error(int code) {
	using errc_t = std::underlying_type_t<std::errc>;
	const auto errc = static_cast<std::errc>(static_cast<errc_t>(code));
	return std::make_error_code(errc);
}


std::error_code lwip_socket_connection::send(std::span<const u8> buffer) {
	int written;
	for (auto it = buffer.begin(); it < buffer.end(); it += written) {
		written = ::send(m_socket.fd, &(*it), buffer.end() - it, 0);
		if (written <= 0) {
			return make_system_error(errno);
		}
	}
	return make_system_error(0);
}

std::error_code lwip_socket_connection::receive(std::span<u8> buffer) {
	int received;
	for (auto it = buffer.begin(); it < buffer.end(); it += received) {
		received = recv(m_socket.fd, &(*it), buffer.end() - it, 0);
		if (received <= 0 && errno != EINTR) {
			return make_system_error(errno);
		}
	}
	return make_system_error(0);
}

void lwip_socket_connection::disconnect() {
	if (m_socket.fd >= 0) {
		shutdown(m_socket.fd, SHUT_RDWR);
	}
}

[[nodiscard]] std::error_code lwip_socket_connection::setReceiveTimeoutInterval(
	const time_t seconds,
	const suseconds_t microseonds
) {
	timeval timeout_interval {
		.tv_sec = seconds,
		.tv_usec = microseonds
	};
	if (setsockopt(m_socket.fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout_interval, sizeof(timeout_interval)) != 0) {
		return make_system_error(errno);
	}
	return make_system_error(0);
}

lwip_safe_fd &lwip_socket_connection::socket() {
	return m_socket;
}

lwip_socket_connection::~lwip_socket_connection() {
	disconnect();
}
