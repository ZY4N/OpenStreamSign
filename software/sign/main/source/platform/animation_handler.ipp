#ifndef INCLUDE_ANIMATION_HANDLER_IMPLEMENTATION
#error Never include this file directly include 'animation_handler.hpp'
#endif

#include <variant>
#include <mutex>

#include <lighting/color.hpp>
#include <platform/WS2815_handler.hpp>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_timer.h>
#include <util/variant_visit.hpp>

template<class animations_t>
void animation_handler<animations_t>::animation_task(void *arg) {

	WS2815_handler leds(static_cast<gpio_num_t>(CONFIG_LED_DATA_PIN), 16);

	auto &[hasNewAnimation, newAnimation] = *static_cast<shared_animation_state*>(arg);

	constexpr auto tickMillis = 1000 / CONFIG_ANIMATION_TICKS_PER_SECOND;

	animation_t currentAnimation;

	std::array<color, 16> colorFrame{};
	std::fill(colorFrame.begin(), colorFrame.end(), colors::black);

	u32 t = 0;

	while (true) {
		const auto startMicro = esp_timer_get_time();

		// Setting the color at the beginning of the tick creates a delay of one tick
		// but insures more accurate color change intervals.
		leds();

		if (hasNewAnimation.test()) {
			
			currentAnimation = newAnimation;
			hasNewAnimation.clear();
			hasNewAnimation.notify_one();
			
			ztu::visit([](auto &animator) {
				animator.init();
			}, currentAnimation.animator);

			t = 0;
		}

		ztu::visit([&](auto &animate) {
			animate(colorFrame, t);
		}, currentAnimation.animator);

		t += currentAnimation.speed;

		std::copy(colorFrame.begin(), colorFrame.end(), leds.begin());

		const auto endMicro = esp_timer_get_time();

		const auto millisUsed = (endMicro - startMicro) / 1000;
		const auto timeLeftInTick = std::max(tickMillis - millisUsed, 0LL);

		vTaskDelay(timeLeftInTick / portTICK_PERIOD_MS);
	}
}

template<class animations_t>
void animation_handler<animations_t>::init(const animation_t& newAnimation) {
	static std::once_flag initFlag;
	std::call_once(initFlag, [&]() {
		shared_state = new shared_animation_state(true, newAnimation);
		xTaskCreate(animation_task, "animation", 4096, shared_state, 5, &animation_task_handle);
	});
}

template<class animations_t>
void animation_handler<animations_t>::setAnimation(const animation_t& newAnimation) {
	shared_state->hasNewAnimation.wait(true);
	shared_state->animation = newAnimation;
	[[maybe_unused]] const auto wasSet = shared_state->hasNewAnimation.test_and_set();
	assert(not wasSet);
}

template<class animations_t>
animation_handler<animations_t>::~animation_handler() {
	vTaskDelete(animation_task_handle);
	delete shared_state;
}