#include <app/main_task.hpp>
#include <domain_logic/sign.hpp>

#include <app/button_task.hpp>
#include <app/setup_task.hpp>
#include <app/sign_task.hpp>

#include <domain_logic/sign_storage.hpp>
#include <domain_logic/sign_animation_controller.hpp>
#include <esp_log.h>

void main_task(void *) {

	if (not sign.storage.open("storage")) {
		ESP_LOGE("MAIN", "[NVS_INIT_ERROR]: nvs init failed");
		return;
	}

	ESP_LOGI("MAIN", "SETTING FLAG");
	sign.storage.set<storage_keys::SETUP_DONE>(false);
	ESP_LOGI("MAIN", "FLAG SET");

	sign.animation_controller.init(sign_state::IDLE);

	sign.switch_task.clear();
	button_task();
	while (true) {
		if (sign.storage.get<storage_keys::SETUP_DONE>()) {
			sign_task();
		} else {
			setup_task();
		}
	}
	vTaskDelete(nullptr);
}
