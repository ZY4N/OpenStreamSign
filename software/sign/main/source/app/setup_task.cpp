#include <app/setup_task.hpp>

#include <domain_logic/sign.hpp>

#include "esp_log.h"
#include <platform/wifi.hpp>
#include <website/config_webserver.hpp>

constexpr auto SETUP_TAG = "SETUP_TASK";

void setup_task() {
	
	sign.animation_controller.setState(sign_state::PROCESSING);

	if (not wifi::ap::create(CONFIG_AP_SSID, CONFIG_AP_PASSWORD, 1, 2)) {
		ESP_LOGE(SETUP_TAG, "[ESP_ERROR]: wifi AP init failed");
		sign.animation_controller.setState(sign_state::IDLE);
		return;
	}

	config_webserver::start();
	sign.animation_controller.setState(sign_state::SETUP);

	sign.switch_task.wait(false);
	sign.switch_task.clear();

	ESP_LOGI(SETUP_TAG, "Stopping server");
	config_webserver::stop();

	ESP_LOGI(SETUP_TAG, "Stopping wifi AP");
	wifi::ap::destroy();

	ESP_LOGI(SETUP_TAG, "Saving");
	// set done byte and save config data to flash memory
	sign.storage.set<storage_keys::SETUP_DONE>(true);
	sign.storage.save();

	sign.animation_controller.setState(sign_state::IDLE);
}

esp_err_t done_post_handler(httpd_req_t *req) {
	ESP_LOGI(SETUP_TAG, "Setup done");

	httpd_resp_send_200(req);

	sign.animation_controller.setState(sign_state::PROCESSING);
	ESP_LOGI(SETUP_TAG, "setting flag");
	// stop setup_task
	[[maybe_unused]] const auto wasSet = sign.switch_task.test_and_set();
	assert(not wasSet);
	sign.switch_task.notify_one();

	return ESP_OK;
}
