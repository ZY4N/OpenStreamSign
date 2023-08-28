#pragma once

#include "lwip_safe_fd.hpp"

#include <util/uix.hpp>
#include <system_error>
#include <span>


using namespace ztu::uix;

/**
 * @class lwip_socket_connection
 *
 * @brief Represents a socket connection for sending and receiving data.
 *
 * The `lwip_socket_connection` class provides a convenient interface for working with sockets. The class provides
 * methods for sending and receiving data over a connected socket, as well as for disconnecting the socket connection.
 * The socket connection is managed using a `safe_fd` object, which is responsible for managing the underlying file
 * descriptor associated with the socket.
 *
 * @note This class is not responsible for creating the socket or establishing the connection.
 */
class lwip_socket_connection {
public:
	lwip_socket_connection() = default;

	lwip_socket_connection(const lwip_socket_connection&) = delete;
	lwip_socket_connection& operator=(const lwip_socket_connection&) = delete;

	lwip_socket_connection(lwip_socket_connection&& other);
	lwip_socket_connection& operator=(lwip_socket_connection&& other);

	/**
	 * @brief Sends data over a connected socket.
	 *
	 * @param buffer The data to send, represented as a span of unsigned 8-bit integers.
	 *
	 * @return A standard error code that indicates the result of the operation. If the operation was successful,
	 * the error code will have a value of 0. Otherwise, the error code will contain a non-zero value indicating the
	 * error that occurred. The error code is accompanied by a system category to provide more context about the error.
	 */
	[[nodiscard]] std::error_code send(std::span<const u8>& data);


	/**
	 * @brief Receives data from a connected socket.
	 *
	 * @param buffer The buffer to store the received data, represented as a span of unsigned 8-bit integers.
	 *
	 * @return A standard error code that indicates the result of the operation. If the operation was successful,
	 * the error code will have a value of 0. Otherwise, the error code will contain a non-zero value indicating the
	 * error that occurred. The error code is accompanied by a system category to provide more context about the error.
	 */
	[[nodiscard]] std::error_code receive(std::span<u8>& data);


	/**
     * @brief Disconnects the socket connection.
	 * 
     * If the socket file descriptor is valid this function disconnects the socket connection
	 * by shutting down both reading and writing channels.
	 *
     * @note This function does not close the socket file descriptor itself.
    */
	void disconnect();


	lwip_safe_fd &socket();

	[[nodiscard]] std::error_code set_send_timeout(u32 milliseconds);

	[[nodiscard]] std::error_code set_receive_timeout(u32 milliseconds);

	~lwip_socket_connection();
	

private:
	lwip_safe_fd m_socket{};
};
