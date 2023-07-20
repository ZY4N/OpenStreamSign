#include <app/button_task.hpp>
#include <domain_logic/sign.hpp>

#include "esp_log.h"

#include <platform/iot_button.h>
#include <cassert>

static void onButtonRelease(void*) {
	[[maybe_unused]] const auto wasSet = sign.switch_task.test_and_set();
	assert(not wasSet);
}

void button_task() {
	auto handle = iot_button_create(static_cast<gpio_num_t>(CONFIG_RESET_BUTTON_PIN), BUTTON_ACTIVE_LOW);
	iot_button_set_evt_cb(handle, BUTTON_CB_RELEASE, onButtonRelease, nullptr);
}
