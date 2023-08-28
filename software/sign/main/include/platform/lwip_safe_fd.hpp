#pragma once

#include "lwip/sockets.h"

struct lwip_safe_fd {

	static constexpr int invalid_fd = -1;

	lwip_safe_fd() = default;

	// Don't allow sockets to be copied
	lwip_safe_fd(const lwip_safe_fd& other) = delete;
	lwip_safe_fd &operator=(const lwip_safe_fd& other) = delete; 


	lwip_safe_fd(int fileDescriptor) : fd{ fileDescriptor } {}

	lwip_safe_fd(lwip_safe_fd&& other) : fd{ other.fd } {
		other.fd = invalid_fd;
	}

	lwip_safe_fd &operator=(lwip_safe_fd&& other) {
		if (&other != this) {
			if (fd != invalid_fd) close(fd);
			fd = other.fd;
			other.fd = invalid_fd;
		}
		return *this;
	}

	~lwip_safe_fd() {
		if (fd >= 0) {
			close(fd);
		}
	}

	int fd{ invalid_fd };
};
