#include <utility>

enum class encoder_state : uint8_t {
	START       = 0,
	CW_FINAL	= 1,
	CW_BEGIN	= 2,
	CW_NEXT		= 3,
	CCW_BEGIN	= 4,
	CCW_FINAL	= 5,
	CCW_NEXT	= 6
};


template<class F>
rotary_encoder<F>::rotary_encoder(gpio_num_t pin_a, gpio_num_t pin_b, F&& on_event) :
	m_pin_a{ pin_a }, m_pin_b{ pin_b }, m_on_event{ std::move(on_event) }
{
	gpio_install_isr_service(0);
    const gpio_config_t config{
      .pin_bit_mask = (1ULL << pin_a) | (1ULL << pin_b),
      .mode = GPIO_MODE_INPUT,
      .pull_up_en = GPIO_PULLUP_ENABLE,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .intr_type = GPIO_INTR_ANYEDGE
    };
    gpio_config(&config);

	constexpr auto handler = +[](void* args) {
		static_cast<rotary_encoder<F>*>(args)->poll();
	};

    gpio_isr_handler_add(pin_a, handler, this);
    gpio_isr_handler_add(pin_b, handler, this);
}



template<class F>
void rotary_encoder<F>::poll() {
	using enum rotation_event;
	using enum encoder_state;

	static constexpr encoder_state event_lookup[7][4] = {
		{ START,    CW_BEGIN,  CCW_BEGIN, START				}, // START
		{ CW_NEXT,  START,     CW_FINAL,  encoder_state(uint8_t(START) | uint8_t(CLOCKWISE))	}, // CW_FINAL
		{ CW_NEXT,  CW_BEGIN,  START,     START				}, // CW_BEGIN
		{ CW_NEXT,  CW_BEGIN,  CW_FINAL,  START				}, // CW_NEXT
		{ CCW_NEXT, START,     CCW_BEGIN, START				}, // CCW_BEGIN
		{ CCW_NEXT, CCW_FINAL, START,     encoder_state(uint8_t(START) | uint8_t(COUNTER_CLOCKWISE))	}, // CCW_FINAL
		{ CCW_NEXT, CCW_FINAL, CCW_BEGIN, START				}, // CCW_NEXT
	};

	const auto new_state = (gpio_get_level(m_pin_b) << 1) | gpio_get_level(m_pin_a);
	m_state = uint8_t(event_lookup[m_state & 0xf][new_state]);

	const auto event = static_cast<rotation_event>(m_state & 0x30);
	if (event != rotation_event::NONE) {
		m_on_event(event);
	}
}

template<class F>
rotary_encoder<F>::~rotary_encoder() {
	gpio_isr_handler_remove(m_pin_a);
    gpio_isr_handler_remove(m_pin_b);
}