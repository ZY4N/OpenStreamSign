#pragma once

#include <lighting/animation.hpp>
#include <atomic>
#include <variant>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

template<class animations_t>
class animation_handler {
public:
	using animation_t = variable_speed_animation<animations_t>;

	void init(const animation_t& newAnimation);

	void setAnimation(const animation_t& newAnimation);

	~animation_handler();

private:
	static void animation_task(void *arg);

	struct shared_animation_state {
		std::atomic_flag hasNewAnimation{ ATOMIC_FLAG_INIT };
		animation_t animation;
	};

	shared_animation_state *shared_state;
	TaskHandle_t animation_task_handle;
};

#define INCLUDE_ANIMATION_HANDLER_IMPLEMENTATION
#include <platform/animation_handler.ipp>
#undef INCLUDE_ANIMATION_HANDLER_IMPLEMENTATION
