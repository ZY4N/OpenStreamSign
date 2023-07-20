#pragma once

#include <util/uix.hpp>
#include <lighting/color.hpp>

#include <array>
#include <vector>

#include <driver/rmt.h>
//#include <driver/rmt_tx.h>
#include <driver/gpio.h>


class rmt_color_converter {
public:
	inline rmt_color_converter &operator=(const color& c);
private:
	std::array<rmt_item32_t, 3 * 8> rmtPattern;
};

class WS2815_handler {
public:
	inline WS2815_handler(gpio_num_t dataPin, usize numPixels, rmt_channel_t rmtChannel = RMT_CHANNEL_0);

	// array interface
	inline rmt_color_converter* begin();
	inline rmt_color_converter* end();
	inline usize size();
	inline rmt_color_converter &operator[](usize index);

	// update colors
	inline bool operator()();

private:
	std::vector<rmt_item32_t> rmtPattern;
	rmt_channel_t rmtChannel;
	usize numPixels;
};

#define INCLUDE_WS2815_HANDLER_IMPLEMENTATION
#include <platform/WS2815_handler.ipp>
#undef INCLUDE_WS2815_HANDLER_IMPLEMENTATION
