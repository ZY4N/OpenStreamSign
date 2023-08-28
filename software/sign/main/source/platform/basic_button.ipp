#include <utility>

#include <esp_log.h>
#include <esp_timer.h>

template<class F>
basic_button<F>::basic_button(gpio_num_t gpio_num, bool pull_up, F&& on_event) :
	m_gpio_num{ gpio_num }, m_pull_up{ pull_up }, m_on_event{ std::move(on_event) }
{
	gpio_install_isr_service(0);
    const gpio_config_t config{
      .pin_bit_mask = (1ULL << gpio_num),
      .mode = GPIO_MODE_INPUT,
      .pull_up_en = m_pull_up ? GPIO_PULLUP_ENABLE : GPIO_PULLUP_DISABLE,
      .pull_down_en = m_pull_up ? GPIO_PULLDOWN_DISABLE : GPIO_PULLDOWN_ENABLE,
      .intr_type = GPIO_INTR_ANYEDGE
    };
    gpio_config(&config);
    gpio_isr_handler_add(gpio_num, +[](void* arg) {
		static_cast<basic_button<F>*>(arg)->poll();
	}, this);
	m_level = gpio_get_level(m_gpio_num);
}




template<class F>
void basic_button<F>::poll() {
	const auto timestamp = esp_timer_get_time();

	// Ignore edges within 200ms for debounce
	if (timestamp - last_timestamp > 200'000) {
		const auto new_level = gpio_get_level(m_gpio_num);
		const auto is_down = (new_level > m_level) == m_pull_up;
		m_on_event(is_down ? basic_button_event::DOWN : basic_button_event::UP);
		m_level = new_level;
	}

	last_timestamp = timestamp;
}

template<class F>
basic_button<F>::~basic_button() {
	gpio_isr_handler_remove(m_gpio_num);
}