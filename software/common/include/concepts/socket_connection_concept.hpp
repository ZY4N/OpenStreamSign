#pragma once

#include <util/uix.hpp>
#include <concepts>
#include <system_error>
#include <span>

namespace socket_connection_concepts {

	using namespace ztu::uix;

	template<class T>
	concept connection_send = requires(T handle, std::span<const u8> data) {
		/**
		 * @brief Sends data over a connected socket.
		 *
		 * @param data The data to be sent, represented as a span of unsigned 8-bit integers.
		 *
		 * @return An 'std::error_code' that indicates the status of the function. Returns '0' on success and a non-zero value on error.
		 */
		{ handle.send(data) } -> std::same_as<std::error_code>;
	};

	template<class T>
	concept connection_receive = requires(T handle, std::span<u8> data) {
		/**
		 * @brief Receives data from a connected socket.
		 *
		 * @param buffer The buffer to store the received data in, represented as a span of unsigned 8-bit integers.
		 *
		 * @return An 'std::error_code' that indicates the status of the function. Returns '0' on success and a non-zero value on error.
		 */
		{ handle.receive(data) } -> std::same_as<std::error_code>;
	};

	template<class T>
	concept connection_disconnect = requires(T handle) {
		/**
		 * @brief Disconnects the socket connection by shutting down both reading and writing channels.
		 *
		 * @note This function does not close the socket file descriptor itself.
		*/
		{ handle.disconnect() } -> std::same_as<void>;
	};
}


template<class T>
concept socket_connection_concept = (
	socket_connection_concepts::connection_send<T> and
	socket_connection_concepts::connection_receive<T> and
	socket_connection_concepts::connection_disconnect<T>
);
