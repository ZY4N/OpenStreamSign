#include <platform/asio_socket_connection.hpp>
#include <atomic>
#include <iostream>

using tcp = asio::ip::tcp;

asio_socket_connection::asio_socket_connection() :
	ctx{ }, socket{ ctx } {};

std::error_code asio_socket_connection::connect(const std::string_view &host, const uint16_t port) {
	using namespace std::chrono_literals;
	std::error_code error;

	const auto ip = asio::ip::address::from_string(host.data(), error);
	if (error) return error;

	auto endpoint = tcp::endpoint(ip, port);

	asio::steady_timer timer(socket.get_executor());
	std::atomic_flag done = ATOMIC_FLAG_INIT;

	socket.async_connect(endpoint, [&](const std::error_code& ec) {
		error = ec;
		timer.cancel();
		done.test_and_set();
		done.notify_one();
	});

	ctx.restart();
	timer.expires_from_now(10s);
	ctx.run();

	timer.async_wait([&](const asio::error_code&) {
		socket.cancel();
		error = asio::error::timed_out;
		done.test_and_set();
		done.notify_one();
	});

	done.wait(false);

	return error;
}



std::error_code asio_socket_connection::listen(const uint16_t port) {
	std::error_code error;

	const auto endpoint = tcp::endpoint(tcp::v4(), port);
	tcp::acceptor acceptor(ctx, endpoint);

	return acceptor.accept(socket, error);
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

namespace detail {
	// https://stackoverflow.com/a/293012/16770597
	// Helper template for implementing timeval options.
	template <int Level, int Name>
	class timeval
	{
	public:
	  // Default constructor.
	  timeval()
		: value_(zero_timeval())
	  {
	  }

	  // Construct with a specific option value.
	  explicit timeval(::timeval v)
		: value_(v)
	  {
	  }

	  // Set the value of the timeval option.
	  timeval& operator=(::timeval v)
	  {
		value_ = v;
		return *this;
	  }

	  // Get the current value of the timeval option.
	  ::timeval value() const
	  {
		return value_;
	  }

	  // Get the level of the socket option.
	  template <typename Protocol>
	  int level(const Protocol&) const
	  {
		return Level;
	  }

	  // Get the name of the socket option.
	  template <typename Protocol>
	  int name(const Protocol&) const
	  {
		return Name;
	  }

	  // Get the address of the timeval data.
	  template <typename Protocol>
	  ::timeval* data(const Protocol&)
	  {
		return &value_;
	  }

	  // Get the address of the timeval data.
	  template <typename Protocol>
	  const ::timeval* data(const Protocol&) const
	  {
		return &value_;
	  }

	  // Get the size of the timeval data.
	  template <typename Protocol>
	  std::size_t size(const Protocol&) const
	  {
		return sizeof(value_);
	  }

	  // Set the size of the timeval data.
	  template <typename Protocol>
	  void resize(const Protocol&, std::size_t s)
	  {
		if (s != sizeof(value_))
		  throw std::length_error("timeval socket option resize");
	  }

	private:
	  static ::timeval zero_timeval()
	  {
		::timeval result = {};
		return result;
	  }

	  ::timeval value_;
	};
}

std::error_code asio_socket_connection::setReceiveTimeoutInterval(const int seconds) {
	std::error_code error;
	timeval timeout_interval {
		.tv_sec = seconds,
		.tv_usec = 0
	};
	socket.set_option(detail::timeval<SOL_SOCKET, SO_RCVTIMEO>{ timeout_interval }, error);
	return error;
}
