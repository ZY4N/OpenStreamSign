#pragma once

#include <platform/safe_json.hpp>
#include <domain_logic/sign_animation.hpp>

namespace json_sign_scaler_detail {
	using namespace json::safe;
	using namespace default_values;

	template<string DefaultValue>
	using json_sign_scaler_type = enumeration<DefaultValue,
		"PING_PONG",
		"SINUS",
		"RANDOM",
		"SMOOTH_RANDOM"
	>;

	struct json_enum_sign_scaler_converter {
		using x_t = u16;
		using y_t = sign_scaler;

		static std::optional<y_t> convert(const x_t &x) {
			std::optional<y_t> y;
			switch (x) {
				case 0: return y.emplace( std::in_place_type<sign_scalers::ping_pong>		);
				case 1: return y.emplace( std::in_place_type<sign_scalers::sinus>			);
				case 2: return y.emplace( std::in_place_type<sign_scalers::random>			);
				case 3: return y.emplace( std::in_place_type<sign_scalers::smooth_random>	);
				default: y = std::nullopt;
			}
			return y;
		};

		static std::optional<x_t> revert(const y_t &src) {
			return {
				static_cast<u16>(src.index())
			};
		};
	};

	template<string DefaultValue>
	using json_sign_scaler_adapter = adapter<json_sign_scaler_type<DefaultValue>, sign_scaler, json_enum_sign_scaler_converter>;

	template<string DefaultValue>
	struct json_sign_scaler : json_sign_scaler_adapter<DefaultValue> {};
}

namespace json::safe::default_values {
	template<string DefaultValue>
	struct registerDefaultValue<json_sign_scaler_detail::json_sign_scaler<DefaultValue>> : public type_container<ADAPTER> {};
}

template<json_sign_scaler_detail::string DefaultValue>
using json_sign_scaler = json_sign_scaler_detail::json_sign_scaler<DefaultValue>;
