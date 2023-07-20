#pragma once

#include <util/uix.hpp>

enum class sign_state : u8 {
	CONNECTED,
	RECORDING,
	RECORDING_PAUSED,
	STREAMING,
	STREAMING_PAUSED,
	IDLE,
	PROCESSING,
	SETUP,

	LAST
};
