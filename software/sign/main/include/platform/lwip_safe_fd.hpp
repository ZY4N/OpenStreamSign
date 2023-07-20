#pragma once

#include "lwip/sockets.h"

struct lwip_safe_fd {

	// Don't allow sockets to be copied
	lwip_safe_fd(const lwip_safe_fd& other) = delete;
	lwip_safe_fd &operator=(const lwip_safe_fd& other) = delete; 


	lwip_safe_fd(int fileDescriptor) : fd{ fileDescriptor } {}

	lwip_safe_fd(lwip_safe_fd&& other) : fd{ other.fd } {
		other.fd = -1;
	}


	lwip_safe_fd &operator=(lwip_safe_fd&& other) {
		if (fd >= 0) close(fd);
		fd = other.fd;
		other.fd = -1;
		return *this;
	}

	~lwip_safe_fd() {
		if (fd >= 0) {
			close(fd);
		}
	}

	int fd{ -1 };
};
