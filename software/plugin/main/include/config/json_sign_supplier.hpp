#pragma once

#include <platform/safe_json.hpp>
#include <domain_logic/sign_animation.hpp>

namespace json_sign_supplier_detail {
	using namespace json::safe;
	using namespace default_values;

	template<json_default_variant DefaultVariant_t>
	struct variant_sign_supplier_converter {
		using x_t = DefaultVariant_t::type;
		using y_t = sign_supplier;

		static std::optional<y_t> convert(const x_t &x) {
			std::optional<y_t> y;
			switch (x.index()) {
				case 0: {
					y.emplace(
						std::in_place_type<sign_suppliers::random>
					);
					break;
				}
				case 1: {
					const auto &value = x.template get<"SEQUENCE">();
					y.emplace(
						std::in_place_type<sign_suppliers::sequence>,
						value.template get<"colors">()
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
				case 0: {
					x.emplace(in_place_name<"RANDOM">);
					break;
				}
				case 1: {
					const auto &value = std::get<sign_suppliers::sequence>(y);
					const auto colors = value.colors();
					x.emplace(
						in_place_name<"SEQUENCE">,
						std::vector<color>{ colors.begin(), colors.end() }
					);
					break;
				}
				default: x = std::nullopt;
			}
			return x;
		};
	};

	template<json_default_variant DefaultVariant_t>
	using string_color_array_adapter = adapter<DefaultVariant_t, sign_supplier, variant_sign_supplier_converter<DefaultVariant_t>>;


	template<json_default_variant auto DefaultVariant>
	struct json_sign_supplier : string_color_array_adapter<decltype(DefaultVariant)> {
		constexpr json_sign_supplier() : string_color_array_adapter<decltype(DefaultVariant)>{ DefaultVariant }{}
	};
}

namespace json::safe::default_values {
	template<json_default_variant auto DefaultVariant>
	struct registerDefaultValue<json_sign_supplier_detail::json_sign_supplier<DefaultVariant>> : public type_container<ADAPTER> {};
}

template<
	json_sign_supplier_detail::string DefaultKey,
	json_sign_supplier_detail::named_alternative... Alternatives
>
using json_sign_supplier = json_sign_supplier_detail::json_sign_supplier<
	json_sign_supplier_detail::variant<DefaultKey, Alternatives...>{}
>;
