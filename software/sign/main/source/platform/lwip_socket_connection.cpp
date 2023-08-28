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

std::error_code lwip_socket_connection::send(std::span<const u8>& bytes_left) {
	while (not bytes_left.empty()) {
		const auto sent = ::send(m_socket.fd, bytes_left.data(), bytes_left.size(), 0);
		if (sent <= 0) return make_system_error(errno);
		bytes_left = bytes_left.subspan(sent);
	}
	return make_system_error(0);
}

std::error_code lwip_socket_connection::receive(std::span<u8>& bytes_left) {
	while (not bytes_left.empty()) {
		const auto received = recv(m_socket.fd, bytes_left.data(), bytes_left.size(), 0);
		// TODO Check if ingoring EINTR can exceed timeout.
		if (received <= 0 and errno != EINTR) return make_system_error(errno);
		bytes_left = bytes_left.subspan(received);
	}
	return make_system_error(0);
}

void lwip_socket_connection::disconnect() {
	if (m_socket.fd >= 0) {
		shutdown(m_socket.fd, SHUT_RDWR);
	}
}


[[nodiscard]] std::error_code lwip_socket_connection::set_send_timeout(const u32 milliseconds) {
	timeval timeout_interval {
		.tv_sec = milliseconds / 1000,
		.tv_usec = static_cast<suseconds_t>((milliseconds % 1000) * 1000)
	};
	if (setsockopt(m_socket.fd, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout_interval, sizeof(timeout_interval)) != 0) {
		return make_system_error(errno);
	}
	return make_system_error(0);
}

[[nodiscard]] std::error_code lwip_socket_connection::set_receive_timeout(const u32 milliseconds) {
	timeval timeout_interval {
		.tv_sec = milliseconds / 1000,
		.tv_usec = static_cast<suseconds_t>((milliseconds % 1000) * 1000)
	};
	if (setsockopt(m_socket.fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout_interval, sizeof(timeout_interval)) != 0) {
		return make_system_error(errno);
	}
	return make_system_error(0);
}

lwip_safe_fd &lwip_socket_connection::socket() {
	return m_socket;
}

lwip_socket_connection::lwip_socket_connection(lwip_socket_connection&& other) {
	m_socket = std::move(other.m_socket);
}

lwip_socket_connection& lwip_socket_connection::operator=(lwip_socket_connection&& other) {
	if (&other != this) {
		disconnect();
		m_socket = std::move(other.m_socket);
	}
	return *this;
}

lwip_socket_connection::~lwip_socket_connection() {
	disconnect();
}
