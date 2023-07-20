#pragma once

#include <concepts/socket_connection_concept.hpp>
#include <platform/asio_socket_connection.hpp>

static_assert(socket_connection_concept<asio_socket_connection>);

using socket_connection = asio_socket_connection;
