#pragma once

#include <platform/safe_json.hpp>
#include <util/uix.hpp>
#include "json_sign_supplier.hpp"
#include "json_sign_mix_type.hpp"

namespace json_sign_sequencer_detail {
	using namespace json::safe;
	using namespace default_values;


	template<json_default_object DefaultObject_t>
	struct json_sign_sequencer_converter {
		using x_t = DefaultObject_t::type;
		using y_t = sign_sequencer;

		static std::optional<y_t> convert(const x_t &x) {
			return y_t{
				x.template get<"supplier">(),
				static_cast<sign_mix_type>(x.template get<"mixType">())
			};
		};

		static std::optional<x_t> revert(const y_t &y) {
			return x_t{
				y.supplier,
				static_cast<u16>(y.mixType)
			};
		};
	};

	template<json_default_object DefaultObject_t>
	using json_sign_sequencer_adapter = adapter<DefaultObject_t, sign_sequencer, json_sign_sequencer_converter<DefaultObject_t>>;


	template<json_default_object auto DefaultObject>
	struct json_sign_sequencer : json_sign_sequencer_adapter<decltype(DefaultObject)> {
		constexpr json_sign_sequencer() : json_sign_sequencer_adapter<decltype(DefaultObject)>{ DefaultObject }{}
	};
}

namespace json::safe::default_values {
	template<json_default_object auto DefaultObject>
	struct registerDefaultValue<json_sign_sequencer_detail::json_sign_sequencer<DefaultObject>> : public type_container<ADAPTER> {};
}

template<json_sign_sequencer_detail::named_value... Entries>
using json_sign_sequencer = json_sign_sequencer_detail::json_sign_sequencer<json_sign_sequencer_detail::object<Entries...>{}>;
