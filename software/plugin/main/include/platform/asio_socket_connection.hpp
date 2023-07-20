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
	
	[[nodiscard]] std::error_code connect(const std::string& host, uint16_t port);
	
	[[nodiscard]] std::error_code send(std::span<const uint8_t> data);
	
	[[nodiscard]] std::error_code receive(std::span<uint8_t> data);
	
	void disconnect();

	[[nodiscard]] bool isConnected() const;

	~asio_socket_connection();

private:
	using tcp = asio::ip::tcp;
	asio::io_service ctx;
	tcp::socket socket;
};
