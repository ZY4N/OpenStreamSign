#pragma once

#include <driver/gpio.h>

enum class rotation_event : uint8_t {
	NONE = 0x0 ,  // No complete step yet.
	CLOCKWISE =   0x10,  // Clockwise step.
	COUNTER_CLOCKWISE =  0x20  // Anti-clockwise step.
};

template<class F>
class rotary_encoder {
public:
	
	rotary_encoder(gpio_num_t pin_a, gpio_num_t pin_b, F&& on_event);

	void poll();

	~rotary_encoder();

private:
	gpio_num_t m_pin_a, m_pin_b;
	F m_on_event;
	uint8_t m_state{ 0 };
};

#include <platform/rotary_encoder.ipp>
