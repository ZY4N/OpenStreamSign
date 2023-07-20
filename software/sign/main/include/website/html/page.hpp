#pragma once

#include "util/string_literal.hpp"

namespace html::page {
	template<string_literal Title, string_literal Script, string_literal Css, size_t N>
	constexpr auto createDefault(const string_literal<N> &body) {
		using namespace ztu::string_literals;
		constexpr auto beforeBody = 
			"<!doctype html><html lang='en'><head><meta charset='utf-8'><title>"_sl +
			Title + "</title><script src="_sl + Script +
			"></script><link rel='stylesheet' type='text/css' href="_sl + Css +
			"></head><body>"_sl;
		
		constexpr auto afterBody = "</body></html>"_sl;
		return beforeBody + body + afterBody;
	}
}
