#pragma once

#include <utility>

namespace ztu {
	__attribute__((always_inline)) inline void visit(auto&& f, auto &&v) {

		using variant_t = std::remove_cvref_t<decltype(v)>;
		constexpr auto numAlternatives = std::variant_size_v<variant_t>;

		[&]<std::size_t... Indices>(std::index_sequence<Indices...>) {
			const auto visitAlternative = [&]<std::size_t Index>() {
				if (const auto ptr = std::get_if<Index>(&v); ptr) {
					f(*ptr);
					return true;
				}
				return false;
			};

			(visitAlternative.template operator()<Indices>() || ...);

		}(std::make_index_sequence<numAlternatives>());
	}

	__attribute__((always_inline)) inline void indexed_visit(auto&& f, auto &&v) {

		using variant_t = std::remove_cvref_t<decltype(v)>;
		constexpr auto numAlternatives = std::variant_size_v<variant_t>;

		[&]<std::size_t... Indices>(std::index_sequence<Indices...>) {
			const auto visitAlternative = [&]<std::size_t Index>() {
				if (const auto ptr = std::get_if<Index>(&v); ptr) {
					f.template operator()<Index>(*ptr);
					return true;
				}
				return false;
			};

			(visitAlternative.template operator()<Indices>() || ...);

		}(std::make_index_sequence<numAlternatives>());
	}
}