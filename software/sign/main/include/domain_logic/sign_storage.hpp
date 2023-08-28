#pragma once

#include <platform/nvs_handler.hpp>
#include <domain_logic/sign_animation.hpp>

enum class storage_keys : u8 {
	SSID, PASSWORD,

	IP_ADDRESS, SUBNETMASK, GATEWAY,

	PORT,

	SETUP_DONE,

	SECRET,

	IDLE_ANIMATION, SETUP_ANIMATION
};

using sign_storage_t = nvs_handler<
	nvs_entry<storage_keys::SSID			, default_types::string_t<CONFIG_SSID_MAX_LEN, [](){
		return ztu::string_literal<CONFIG_SSID_MAX_LEN + 1>{ "lolmaorofl" };
	}>>{},
	
	nvs_entry<storage_keys::PASSWORD		, default_types::string_t<CONFIG_PASSWORD_MAX_LEN, [](){
		return ztu::string_literal<CONFIG_PASSWORD_MAX_LEN + 1>{ "jshvfjas"};
	}>>{},

	nvs_entry<storage_keys::IP_ADDRESS		, default_types::u32_t<[]() -> u32 { return 1; }>>{},
	nvs_entry<storage_keys::SUBNETMASK		, default_types::u32_t<[]() -> u32 { return 1; }>>{},
	nvs_entry<storage_keys::GATEWAY			, default_types::u32_t<[]() -> u32 { return 1; }>>{},

	nvs_entry<storage_keys::PORT			, default_types::u16_t<[]() -> u16 { return 1; }>>{},

	nvs_entry<storage_keys::SETUP_DONE		, default_types::u8_t<[]() -> u8 { return 1; }>>{},

	nvs_entry<storage_keys::SECRET			, default_types::array_t<uint8_t, 64 + 32, []() -> std::array<u8, 64 + 32> { return {}; }>>{},

	nvs_entry<storage_keys::IDLE_ANIMATION	, default_types::object_t<sign_animation,
		[]() -> sign_animation{
			return sign_animation{
				sign_animations::moving_colors{
					{
						sign_suppliers::sequence{
							colors::red, colors::yellow,  
							colors::green, colors::turquoise,
							colors::blue, colors::pink
						},
						color_mixing::type::LINEAR_INTERPOLATION
					},
					256,
					8
				},
				0.1f
			};
		}
	>>{},
	nvs_entry<storage_keys::SETUP_ANIMATION, default_types::object_t<
		sign_animation,
		[]() -> sign_animation {
			return sign_animation{
				sign_animations::moving_colors{
					{
						{
							sign_suppliers::sequence{
								colors::blue, colors::pink
							}
						},
						color_mixing::type::LINEAR_INTERPOLATION
					},
					1,
					8
				},
				2.0f
			};
		}
	>>{}
>;
