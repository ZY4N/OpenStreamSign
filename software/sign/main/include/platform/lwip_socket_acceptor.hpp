#pragma once

#include <system_error>

#include "lwip_socket_connection.hpp"

/**
 * @class socket_acceptor
 * 
 * @brief A class that represents a socket acceptor.
 * 
 * This class provides a simple interface for accepting incoming connections and initializing a `socket_connection` instance.
 * It provides a `initialize` method for creating a socket with the specified address family and port,
 * and a `listen` method for waiting for incoming connections and initializing a `socket_connection` instance.
 */
class lwip_socket_acceptor {
public:
	lwip_socket_acceptor() = default;


	/**
	 * @brief Initialize the socket acceptor
	 * 
	 * This function creates a socket with the specified address family and port.
	 * If the address family is AF_INET, it sets the IP protocol to IPPROTO_IP.
	 * The socket is then bound to the specified address and port and set to listen.
	 * 
	 * @param addr_family		The address family of the socket
	 * @param port				The port to bind the socket to
	 * 
	 * @return std::error_code indicating the result of the operation. Zero on success, non-zero on error.
	 */
	[[nodiscard]] std::error_code initialize(const uint16_t port);

	
	/**
	 * @brief This function waits for an incoming connection and initializes the given 'socket_connection` instance.
	 *
	 * @param connection `socket_connection` instance to store the newly created socket in.
	 *
	 * @return std::error_code indicating the result of the operation. Zero on success, non-zero on error.
	 */
	[[nodiscard]] std::error_code listen(
		lwip_socket_connection &connection,
		uint32_t timeout_ms = 0
	);


private:
	lwip_safe_fd m_socket{};
};
