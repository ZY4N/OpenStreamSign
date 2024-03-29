//
// Created by zy4n on 23.04.23.
//
#pragma once

#define ASIO_STANDALONE

#include <platform/asio.hpp>
#include <asio/ts/internet.hpp>

class asio_socket_connection {
public:
	asio_socket_connection();

	[[nodiscard]] std::error_code listen(uint16_t port);
	
	[[nodiscard]] std::error_code connect(const std::string_view& host, uint16_t port);
	
	[[nodiscard]] std::error_code send(std::span<const uint8_t> data);
	
	[[nodiscard]] std::error_code receive(std::span<uint8_t> data);
	
	void disconnect();

	[[nodiscard]] bool isConnected() const;

	[[nodiscard]] std::error_code setReceiveTimeoutInterval(int seconds);

	~asio_socket_connection();

private:
	using tcp = asio::ip::tcp;
	asio::io_service ctx;
	tcp::socket socket;
};
// socket.set_option(boost::asio::detail::socket_option::integer<SOL_SOCKET, SO_RCVTIMEO>{ 200 });