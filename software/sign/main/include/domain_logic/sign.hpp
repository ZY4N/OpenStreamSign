#pragma once

#include "sign_animation_controller.hpp"
#include "sign_storage.hpp"
#include <atomic>
#include <system_error>

struct sign_t {
	// handle for accessing flash memory
	sign_storage_t storage;
	// animation controller for LED strip
	sign_animation_controller_t animation_controller;
	// flag to change between setup and sign mode
	std::atomic_flag switch_task;
	// last major error
	std::error_code error;
};

extern sign_t sign;
