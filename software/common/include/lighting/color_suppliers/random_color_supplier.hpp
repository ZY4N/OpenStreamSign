#pragma once

#include <util/uix.hpp>
#include <util/hash_u32.hpp>
#include "../color.hpp"
#include <fill_random.hpp>

namespace color_suppliers_detail {

	using namespace ztu::uix;

	struct random_color_supplier {

		constexpr random_color_supplier() = default;

		void init() {
			const auto begin = reinterpret_cast<u8*>(&salt); 
			fill_random({ begin, begin + sizeof(salt) });
		}

		color operator()(const u32 t) {
			union {
				u32 integer;
				color c;
			} itc { .integer = ztu::hash_u32(salt + t) };
			return itc.c;
		}

		constexpr bool operator==(const random_color_supplier &) const {
			return true;
		}
		
		u32 salt{ 0 };
	};
	
}
