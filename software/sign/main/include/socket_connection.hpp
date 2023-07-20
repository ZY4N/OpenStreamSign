#pragma once

#include <concepts/socket_connection_concept.hpp>
#include <platform/lwip_socket_connection.hpp>

static_assert(socket_connection_concept<lwip_socket_connection>);

using socket_connection = lwip_socket_connection;
