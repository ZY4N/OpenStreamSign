#include <platform/asio_socket_connection.hpp>

using tcp = asio::ip::tcp;

asio_socket_connection::asio_socket_connection() :
	ctx{ }, socket{ ctx } {};

std::error_code asio_socket_connection::connect(const std::string& host, const uint16_t port) {
	std::error_code error;

	const auto ip = asio::ip::address::from_string(host, error);
	if (error) return error;

	auto endpoint = tcp::endpoint(ip, port);
	socket.connect(endpoint, error);

	return error;
}

std::error_code asio_socket_connection::listen(const uint16_t port) {
	std::error_code error;

	const auto endpoint = tcp::endpoint(tcp::v4(), port);
	tcp::acceptor acceptor(ctx, endpoint);
	acceptor.accept(socket, error);

	return error;
}

std::error_code asio_socket_connection::send(std::span<const uint8_t> data) {
	std::error_code error;
	asio::write(socket, asio::const_buffer{ data.data(), data.size() }, error);
	return error;
}

std::error_code asio_socket_connection::receive(std::span<uint8_t> data) {
	std::error_code error;
	asio::read(socket, asio::mutable_buffer{ data.data(), data.size() }, error);
	return error;
}

void asio_socket_connection::disconnect() {
	if (isConnected()) {
		socket.close();
	}
}

bool asio_socket_connection::isConnected() const {
	return socket.is_open();
}

asio_socket_connection::~asio_socket_connection() {
	socket.close();
	ctx.stop();
}
