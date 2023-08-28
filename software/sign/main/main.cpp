#include <app/main_task.hpp>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

extern "C" {
	void app_main(void) {
		//create extra thread to controll stack size
		xTaskCreate(main_task, "MAIN_TASK", 8192, nullptr, 1, nullptr);
	}
}

/*
#include <platform/animation_handler.hpp>
#include <domain_logic/sign_animation.hpp>

using sign_animation_handler_t = animation_handler<sign_animations>;

extern "C" {
	void app_main(void) {
		sign_animation_handler_t animationHandler;

		sign_animation animation1{
			sign_basic_animation{
				sign_animations::moving_colors{
					sign_sequencer{
						sign_supplier{
							sign_suppliers::sequence{
								colors::red, colors::green
							}
						},
						color_mixing::type::LINEAR_INTERPOLATION
					}
				}
			},
			1
		};

		sign_animation animation2{
			sign_basic_animation{
				sign_animations::moving_colors{
					sign_sequencer{
						sign_supplier{
							sign_suppliers::sequence{
								colors::white, colors::blue
							}
						},
						color_mixing::type::FADE_IN_OUT
					}
				}
			},
			1
		};

		animationHandler.init(animation1);

		bool isRed = false;
		while (true) {
			animationHandler.setAnimation(isRed ? animation1 : animation2);
			vTaskDelay(10000 / portTICK_PERIOD_MS);
			isRed ^= true;
		}

		*WS2815_handler leds(static_cast<gpio_num_t>(CONFIG_LED_DATA_PIN), 16);

		constexpr auto tickMillis = 1000 / 1;

		bool isRed = true;
		while (true) {
			std::fill(leds.begin(), leds.end(), isRed ? colors::red : colors::yellow);
			leds();
			vTaskDelay(tickMillis / portTICK_PERIOD_MS);
			isRed ^= true;
		}*
	}
}
*/
/*
#ifdef PPPPPP
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "esp_log.h"

#include "platform/rotary_encoder.h"

#define TAG "app"

#define ROT_ENC_A_GPIO ((gpio_num_t)18)
#define ROT_ENC_B_GPIO ((gpio_num_t)19)

#define ENABLE_HALF_STEPS false  // Set to true to enable tracking of rotary encoder at half step resolution
#define RESET_AT          0      // Set to a positive non-zero number to reset the position if this value is exceeded
#define FLIP_DIRECTION    false  // Set to true to reverse the clockwise/counterclockwise sense

extern "C" {
void app_main()
{
    // esp32-rotary-encoder requires that the GPIO ISR service is installed before calling rotary_encoder_register()
    ESP_ERROR_CHECK(gpio_install_isr_service(0));

    // Initialise the rotary encoder device with the GPIOs for A and B signals
    rotary_encoder_info_t info { };
    ESP_ERROR_CHECK(rotary_encoder_init(&info, ROT_ENC_A_GPIO, ROT_ENC_B_GPIO));
    ESP_ERROR_CHECK(rotary_encoder_enable_half_steps(&info, ENABLE_HALF_STEPS));
#ifdef FLIP_DIRECTION
    ESP_ERROR_CHECK(rotary_encoder_flip_direction(&info));
#endif

    // Create a queue for events from the rotary encoder driver.
    // Tasks can read from this queue to receive up to date position information.
    QueueHandle_t event_queue = rotary_encoder_create_queue();
    ESP_ERROR_CHECK(rotary_encoder_set_queue(&info, event_queue));

    while (1)
    {
        // Wait for incoming events on the event queue.
        rotary_encoder_event_t event = { 0 };
        if (xQueueReceive(event_queue, &event, 1000 / portTICK_PERIOD_MS) == pdTRUE)
        {
            ESP_LOGI(TAG, "Event: position %d, direction %s", int(event.state.position),
                     event.state.direction ? (event.state.direction == ROTARY_ENCODER_DIRECTION_CLOCKWISE ? "CW" : "CCW") : "NOT_SET");
        }
        else
        {
            // Poll current position and direction
            rotary_encoder_state_t state = { 0 };
            ESP_ERROR_CHECK(rotary_encoder_get_state(&info, &state));
            ESP_LOGI(TAG, "Poll: position %d, direction %s", int(state.position),
                     state.direction ? (state.direction == ROTARY_ENCODER_DIRECTION_CLOCKWISE ? "CW" : "CCW") : "NOT_SET");

            // Reset the device
            if (RESET_AT && (state.position >= RESET_AT || state.position <= -RESET_AT))
            {
                ESP_LOGI(TAG, "Reset");
                ESP_ERROR_CHECK(rotary_encoder_reset(&info));
            }
        }
    }
    ESP_LOGE(TAG, "queue receive failed");

    ESP_ERROR_CHECK(rotary_encoder_uninit(&info));
}
}
*/
/*
#include <platform/basic_button.hpp>
#include <platform/rotary_encoder.hpp>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

constexpr auto TAG = "MAIN_TASK";

extern "C" {
	void app_main(void) {

		auto ticksPressed = 0ULL;
		bool buttonPressed = false;
		const auto btn = basic_button((gpio_num_t)21, false, [&buttonPressed](basic_button_event e) {
			buttonPressed = e == basic_button_event::DOWN;
		});

		int value = 0, old_value = 0;
		const auto encoder = rotary_encoder((gpio_num_t)18, (gpio_num_t)19, [&value](rotation_event e) {
			value += e == rotation_event::CLOCKWISE ? 1 : -1;
		});

		while (true) {
			if (value != old_value) {
				ESP_LOGI(TAG, "value: %d", value);
				old_value = value;
			}
			if (buttonPressed) {
				ESP_LOGI(TAG, "pressed: %llu", ticksPressed);
				ticksPressed++;
			} else {
				ticksPressed = 0;
			}
			vTaskDelay(1);
		}

	}
}
*/
/*
#include <platform/button.hpp>
#include <platform/rotary_encoder.hpp>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

constexpr auto TAG = "MAIN_TASK";

extern "C" {
	void app_main(void) {

		const auto pin_a = (gpio_num_t)18, pin_b = (gpio_num_t)19;

		const gpio_config_t config{
			.pin_bit_mask = (1ULL << pin_a) | (1ULL << pin_b),
			.mode = GPIO_MODE_INPUT,
			.pull_up_en = GPIO_PULLUP_ENABLE,
			.pull_down_en = GPIO_PULLDOWN_DISABLE,
			.intr_type = GPIO_INTR_ANYEDGE
		};
		gpio_config(&config);



		while (true) {
			const auto va = gpio_get_level(pin_a);
			const auto vb = gpio_get_level(pin_b);

			ESP_LOGI(TAG, "%c %c", va ? '#' : '|', vb ? '#' : '|');
			
			vTaskDelay(1);
		}

	}
}

#endif
*/

