#pragma once

#include <driver/gpio.h>


enum class basic_button_event {
	DOWN, UP
};

template<class F>
class basic_button {
public:
	
	basic_button(gpio_num_t gpio_num, bool pullUp, F&& on_event);

	void poll();

	~basic_button();

private:
	gpio_num_t m_gpio_num;
	bool m_pull_up; 
	F m_on_event;
	int m_level{ 0 };
	int32_t last_timestamp{ 0 };
};


#include <platform/basic_button.ipp>
