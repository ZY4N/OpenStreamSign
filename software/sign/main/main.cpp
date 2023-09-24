#include <app/main_task.hpp>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

extern "C" {
	void app_main(void) {
		//create extra thread to controll stack size
		xTaskCreate(main_task, "MAIN_TASK", 16384 , nullptr, 1, nullptr);
	}
}
