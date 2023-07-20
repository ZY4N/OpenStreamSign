#pragma once

#include <system_error>

#include <platform/safe_json.hpp>

#include "json_sign_animation.hpp"

namespace notifier_default_config {

	using namespace json::safe::default_builder;

	constexpr auto default_sign_scaler = json_sign_scaler<"SINUS"_S>{};

	constexpr auto default_sign_mix_type = json_sign_mix_type<"LINEAR_INTERPOLATION"_S>{};

	constexpr auto default_sign_supplier = json_sign_supplier<"RANDOM"_S,
		holds<"RANDOM">{},
		holds<"SEQUENCE",
			set<"colors", color_array<"f00 ff0 0f0 0ff 00f f0f"_S>{}>{}
		>{}
	>{};

	constexpr auto default_sign_sequencer = json_sign_sequencer<
		set<"supplier", default_sign_supplier>{},
		set<"mixType", default_sign_mix_type>{}
	>{};

	constexpr auto default_sign_basic_animation = json_sign_basic_animation<"STOP_MOTION"_S, // "MOVING_COLORS"_S,
		holds<"UNIFORM_COLOR",
			set<"sequencer", default_sign_sequencer>{}
		>{},
		holds<"MOVING_COLORS",
			set<"sequencer", default_sign_sequencer>{},
			set<"perPixelOffset", 2_U>{},
			set<"pixelOffset", 0_U>{}
		>{},
		holds<"MOVING_PIXEL",
			set<"sequencer", default_sign_sequencer>{},
			set<"scaler", default_sign_scaler>{},
			set<"colorSpeed", 1.0_N>{},
			set<"width", 1_U>{}
		>{},
		holds<"STOP_MOTION",
			set<"frames",
				color_array<
					"f00 ff0 0f0 0ff 00f f0f f00 ff0 0f0 0ff 00f f0f f00 ff0 0f0 0ff "
					"ff0 0f0 0ff 00f f0f f00 ff0 0f0 0ff 00f f0f f00 ff0 0f0 0ff f00 "
					"0f0 0ff 00f f0f f00 ff0 0f0 0ff 00f f0f f00 ff0 0f0 0ff f00 ff0 "
					"0ff 00f f0f f00 ff0 0f0 0ff 00f f0f f00 ff0 0f0 0ff f00 ff0 0f0 "
					"00f f0f f00 ff0 0f0 0ff 00f f0f f00 ff0 0f0 0ff f00 ff0 0f0 0ff "
					"f0f f00 ff0 0f0 0ff 00f f0f f00 ff0 0f0 0ff f00 ff0 0f0 0ff 00f "
					"f0f f00 ff0 0f0 0ff 00f f0f f00 ff0 0f0 0ff f00 ff0 0f0 0ff 00f "
					"f00 ff0 0f0 0ff 00f f0f f00 ff0 0f0 0ff f00 ff0 0f0 0ff 00f f0f"_S
				>{}
			>{}
		>{}
	>{};

	constexpr auto default_sign_animation = json_sign_animation<
		set<"animation", default_sign_basic_animation>{},
		set<"duration", 1.0_N>{}
	>{};

	constexpr auto default_notifier_config = object<
		set<"animations", object<
			set<"CONNECTED",		default_sign_animation>{},
			set<"RECORDING",		default_sign_animation>{},
			set<"RECORDING_PAUSED",	default_sign_animation>{},
			set<"STREAMING",		default_sign_animation>{},
			set<"STREAMING_PAUSED",	default_sign_animation>{},
			set<"IDLE",				default_sign_animation>{},
			set<"PROCESSING",		default_sign_animation>{},
			set<"SETUP",			default_sign_animation>{}
		>{}>{},
		set<"connection", object<
			set<"ip",		"192.168.2.222"_S>{},
			set<"port",		65025_U>{},
			set<"secret",	"ZwvdH5TokDfYpcr1v67MFEqph3uJfcKlAGB1683v9LSqXDrbijFrOFFu79VWoM6Z7qP82MtCMQ3JU2Jv/n/GG0kknnGPQ4djr3LLxg94RweLduErAPrLZDCborWcU4e+"_S>{},
			set<"timeout_interval_ms", array<
				  object<
					  set<"dt", 		    10'000_U>{},
					  set<"interval", 	 	  1000_U>{}
				  >{},
				  object<
					  set<"dt", 		    60'000_U>{},
					  set<"interval", 	     5'000_U>{}
				  >{},
				  object<
					  set<"dt", 		 1'200'000_U>{},
					  set<"interval", 	    10'000_U>{}
				  >{},
				  object<
					  set<"dt", 		86'400'000_U>{},
					  set<"interval", 	    60'000_U>{}
				  >{}
			>{}>{},
			set<"heartbeat_interval",	10'000_U>{},
			set<"heartbeat_correction",	0.3_N>{}
		>{}>{}
	 >{};
}

using notifier_default_config::default_notifier_config;
using notifier_config_t = decltype(default_notifier_config)::type;
