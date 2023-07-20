#ifndef INCLUDE_WS2815_HANDLER_IMPLEMENTATION
#error Never include this file directly include 'WS2815_handler.hpp'
#endif

#include <cassert>

rmt_color_converter &rmt_color_converter::operator=(const color& c) {
	const auto setBit = [&](const auto index, const auto bit) {
		if (bit) {
			rmtPattern[index] = {
				.duration0 = 10,
				.level0    = 1,
				.duration1 = 6,
				.level1    = 0
			};
		} else {
			rmtPattern[index] =  {
				.duration0 = 4,
				.level0    = 1,
				.duration1 = 8,
				.level1    = 0
			};
		}
	};

	const auto setByte = [&](const auto index, auto byte) {
		const auto bitOffset = index * 8;
		for (auto i = 0U; i < 8U; i++) {
			setBit(bitOffset + i, byte & 0x80);
			byte <<= 1;
		}
	};

	setByte(0, c.g);
	setByte(1, c.r);
	setByte(2, c.b);

	return *this;
}


WS2815_handler::WS2815_handler(gpio_num_t dataPin, usize pixels, rmt_channel_t channel):
	rmtPattern(pixels * 24 + 1), rmtChannel{ channel }, numPixels{ pixels }
{
	std::fill(begin(), end(), colors::black);

	rmtPattern.back() = {
		.duration0 = 0,
		.level0    = 0,
		.duration1 = 0,
		.level1    = 0
	};

	rmt_config_t rmtConfig {
		.rmt_mode = RMT_MODE_TX,
		.channel = rmtChannel,
		.gpio_num = dataPin,
		.clk_div = 8,
		.mem_block_num = static_cast<uint8_t>(8 - rmtChannel),
		.flags = 0,
		.tx_config = {
			.carrier_freq_hz = 10000,
			.carrier_level = RMT_CARRIER_LEVEL_HIGH,
			.idle_level = RMT_IDLE_LEVEL_LOW,
			.carrier_duty_percent = 50,
			.carrier_en = false,
			.loop_en = false,
			.idle_output_en = true			
		}
	};

	rmt_config(&rmtConfig);
	rmt_driver_install(rmtChannel, 0, 0);
}

rmt_color_converter* WS2815_handler::begin() {
	return reinterpret_cast<rmt_color_converter*>(rmtPattern.data());
}

rmt_color_converter* WS2815_handler::end() {
	return begin() + size();
}

usize WS2815_handler::size() {
	return numPixels;
}

rmt_color_converter &WS2815_handler::operator[](usize index) {
	assert(index < size());
	return *(begin() + index);
}

bool WS2815_handler::operator()() {
	return rmt_write_items(rmtChannel, rmtPattern.data(), rmtPattern.size() - 1, 1) == ESP_OK;
}
