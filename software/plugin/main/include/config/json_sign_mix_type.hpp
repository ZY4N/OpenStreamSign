#pragma once

#include <platform/safe_json.hpp>
#include <domain_logic/sign_animation.hpp>

namespace json_sign_mix_type_detail {
	using namespace json::safe;
	using namespace default_values;

	template<string DefaultValue>
	using json_sign_mix_type = enumeration<DefaultValue,
		"LINEAR_INTERPOLATION",
		"FADE_IN_OUT",
		"PWM",
		"RAMP",
		"NO_MIXING"
	>;
}

namespace json::safe::default_values {
	template<string DefaultValue>
	struct registerDefaultValue<json_sign_mix_type_detail::json_sign_mix_type<DefaultValue>> : public type_container<ADAPTER> {};
}

template<json_sign_mix_type_detail::string DefaultValue>
using json_sign_mix_type = json_sign_mix_type_detail::json_sign_mix_type<DefaultValue>;
