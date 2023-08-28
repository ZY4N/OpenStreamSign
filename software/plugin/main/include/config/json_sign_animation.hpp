#pragma once

#include <platform/safe_json.hpp>
#include <domain_logic/sign_animation.hpp>
#include <vector>

#include "json_color_array.hpp"
#include "json_sign_scaler.hpp"
#include "json_sign_sequencer.hpp"

namespace json_sign_animation_detail {
	using namespace json::safe;
	using namespace default_values;

	using sign_basic_animation = sign_animation::animation_t;

	template<json_default_variant DefaultVariant_t>
	struct variant_sign_basic_animation_converter {
		using x_t = DefaultVariant_t::type;
		using y_t = sign_basic_animation;

		static std::optional<y_t> convert(const x_t &x) {
			std::optional<y_t> y;
			switch (x.index()) {
				case *x_t::indexOf("UNIFORM_COLOR"): {
					const auto &value = x.template get<"UNIFORM_COLOR">();
					y.emplace(
						std::in_place_type<sign_animations::uniform_color>,
						value.template get<"sequencer">()
					);
					break;
				}
				case *x_t::indexOf("MOVING_COLORS"): {
					const auto &value = x.template get<"MOVING_COLORS">();
					y.emplace(
						std::in_place_type<sign_animations::moving_colors>,
						value.template get<"sequencer">(),
						static_cast<u32>(value.template get<"perPixelOffset">()),
						static_cast<u32>(value.template get<"pixelOffset">())
					);
					break;
				}
				case *x_t::indexOf("MOVING_PIXEL"): {
					const auto &value = x.template get<"MOVING_PIXEL">();
					y.emplace(
						std::in_place_type<sign_animations::moving_pixel>,
						value.template get<"sequencer">(),
						value.template get<"scaler">(),
						static_cast<float>(value.template get<"colorSpeed">()),
						static_cast<u8>(value.template get<"width">())
					);
					break;
				}
				case *x_t::indexOf("STOP_MOTION"): {
					const auto &value = x.template get<"STOP_MOTION">();
					y.emplace(
						std::in_place_type<sign_animations::stop_motion>,
						value.template get<"frames">()
					);
					break;
				}
				default: y = std::nullopt;
			}
			return y;
		};

		static std::optional<x_t> revert(const y_t &y) {
			std::optional<x_t> x;
			switch (y.index()) {
				case *x_t::indexOf("UNIFORM_COLOR"): {
					const auto &value = std::get<sign_animations::uniform_color>(y);
					x.emplace(
						in_place_name<"UNIFORM_COLOR">,
						value.sequencer
					);
					break;
				}
				case *x_t::indexOf("MOVING_COLORS"): {
					const auto &value = std::get<sign_animations::moving_colors>(y);
					x.emplace(
						in_place_name<"MOVING_COLORS">,
						value.sequencer,
						value.perPixelOffset,
						value.pixelOffset
					);
					break;
				};
				case *x_t::indexOf("MOVING_PIXEL"): {
					const auto &value = std::get<sign_animations::moving_pixel>(y);
					x.emplace(
						in_place_name<"MOVING_PIXEL">,
						value.sequencer,
						value.scaler,
						value.colorSpeed,
						value.width
					);
					break;
				}
				case *x_t::indexOf("STOP_MOTION"): {
					const auto &value = std::get<sign_animations::stop_motion>(y);
					const auto frames = value.frames();
					x.emplace(
						in_place_name<"STOP_MOTION">,
						std::vector<color>{ frames.begin(), frames.end() }
					);
					break;
				}
				default: x = std::nullopt;
			}
			return x;
		};
	};

	template<json_default_variant DefaultVariant_t>
	using variant_sign_basic_animation_adapter = adapter<DefaultVariant_t, sign_basic_animation, variant_sign_basic_animation_converter<DefaultVariant_t>>;

	template<json_default_variant auto DefaultVariant>
	struct json_sign_basic_animation : variant_sign_basic_animation_adapter<decltype(DefaultVariant)> {
		constexpr json_sign_basic_animation() : variant_sign_basic_animation_adapter<decltype(DefaultVariant)>{ DefaultVariant }{}
	};

	template<json_default_object DefaultObject_t>
	struct object_sign_animation_converter {
		using x_t = DefaultObject_t::type;
		using y_t = sign_animation;

		static std::optional<y_t> convert(const x_t &src) {
			return y_t{
				src.template get<"animation">(),
				src.template get<"duration">()
			};
		}

		static std::optional<x_t> revert(const y_t &y) {
			return x_t{
				y.animator,
				y.duration()
			};
		}
	};

	template<json_default_object DefaultObject_t>
	using object_sign_basic_animation_adapter = adapter<DefaultObject_t, sign_animation, object_sign_animation_converter<DefaultObject_t>>;

	template<json_default_object auto DefaultObject>
	struct json_sign_animation : object_sign_basic_animation_adapter<decltype(DefaultObject)> {
		constexpr json_sign_animation() : object_sign_basic_animation_adapter<decltype(DefaultObject)>{ DefaultObject }{}
	};
}

namespace json::safe::default_values {
	template<json_sign_animation_detail::json_default_variant auto DefaultVariant>
	struct registerDefaultValue<json_sign_animation_detail::json_sign_basic_animation<DefaultVariant>> : public type_container<ADAPTER> {};

	template<json_default_object auto DefaultObject>
	struct registerDefaultValue<json_sign_animation_detail::json_sign_animation<DefaultObject>> : public type_container<ADAPTER> {};
}

template<
	json_sign_animation_detail::string DefaultKey,
	json_sign_animation_detail::named_alternative... Alternatives
>
using json_sign_basic_animation = json_sign_animation_detail::json_sign_basic_animation<
    json_sign_animation_detail::variant<DefaultKey, Alternatives...>{}
>;


template<
	json_sign_animation_detail::named_value... Entries
>
using json_sign_animation = json_sign_animation_detail::json_sign_animation<json_sign_animation_detail::object<Entries...>{}>;
