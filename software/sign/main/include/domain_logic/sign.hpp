#pragma once

#include "sign_animation_controller.hpp"
#include "sign_storage.hpp"
#include <atomic>

/*
struct sign {
	inline static sign_storage_t storage{ };
	inline static sign_animation_controller_t animation_controller{ };
	inline static std::atomic_flag switch_task{ ATOMIC_FLAG_INIT };
};
*/
/*
struct sign {
	static sign_storage_t storage;
	static sign_animation_controller_t animation_controller;
	static std::atomic_flag switch_task;
};
*/

struct sign_t {
	sign_storage_t storage;
	sign_animation_controller_t animation_controller;
	std::atomic_flag switch_task;
};

extern sign_t sign;
