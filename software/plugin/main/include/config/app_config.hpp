#pragma once

#include <system_error>

#include <platform/safe_json.hpp>

#include "json_sign_animation.hpp"

namespace app_default_config {

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

	constexpr auto default_sign_uniform_color = holds<"UNIFORM_COLOR",
		set<"sequencer", default_sign_sequencer>{}
	>{};

	constexpr auto default_sign_moving_colors = holds<"MOVING_COLORS",
		set<"sequencer", default_sign_sequencer>{},
		set<"perPixelOffset", 2_U>{},
		set<"pixelOffset", 0_U>{}
	>{};

	constexpr auto default_sign_moving_pixel = holds<"MOVING_PIXEL",
			set<"sequencer", default_sign_sequencer>{},
			set<"scaler", default_sign_scaler>{},
			set<"colorSpeed", 1.0_N>{},
			set<"width", 1_U>{}
		>{};

	constexpr auto default_sign_stop_motion = holds<"STOP_MOTION",
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
	>{};

	constexpr auto default_sign_basic_animation = json_sign_basic_animation<"MOVING_COLORS"_S,
		default_sign_uniform_color,
		default_sign_moving_colors,
		default_sign_moving_pixel,
		default_sign_stop_motion
	>{};

	constexpr auto default_sign_animation = json_sign_animation<
		set<"animation", default_sign_basic_animation>{},
		set<"duration", 1.0_N>{}
	>{};

	template<string Colors>
	constexpr auto default_uniform_color_animation = (
		json_sign_basic_animation<"UNIFORM_COLOR"_S,
			holds<"UNIFORM_COLOR",
				set<"sequencer",
					json_sign_sequencer<
						set<"supplier",
							json_sign_supplier<"SEQUENCE"_S,
								holds<"RANDOM">{},
								holds<"SEQUENCE",
									set<"colors", color_array<Colors>{}>{}
								>{}
							>{}
						>{},
						set<"mixType", default_sign_mix_type>{}
					>{}
				>{}
			>{},
			default_sign_moving_colors,
			default_sign_moving_pixel,
			default_sign_stop_motion
		>{}
	);

	constexpr auto default_app_config = object<
		set<"animations", object<
			set<"CONNECTED", 		default_uniform_color_animation<"fff"_S>>{},
			set<"RECORDING",		default_uniform_color_animation<"f00"_S>>{},
			set<"RECORDING_PAUSED",	default_uniform_color_animation<"f00 ff0"_S>>{},
			set<"STREAMING",		default_uniform_color_animation<"f0f"_S>>{},
			set<"STREAMING_PAUSED",	default_uniform_color_animation<"f0f 00f"_S>>{},
			set<"IDLE",				default_uniform_color_animation<"000"_S>>{},
			set<"PROCESSING",		default_uniform_color_animation<"f0f 0f0"_S>>{},
			set<"SETUP",			default_uniform_color_animation<"ff0"_S>>{}
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

using app_default_config::default_app_config;
using app_config_t = decltype(default_app_config)::type;
