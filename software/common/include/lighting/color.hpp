#pragma once

#include <util/uix.hpp>

struct color {
	ztu::u8 r, g, b;
	constexpr bool operator==(const color&) const = default;
};

namespace colors {
	static constexpr color
		black{ 0, 0, 0 },
		white{ 255, 255, 255 },
		red{ 255, 0, 0 },
		yellow{ 255, 255, 0 },
		green{ 0, 255, 0 },
		turquoise{ 0, 255, 255 },
		blue{ 0, 0, 255 },
		pink{ 255, 0, 255 };
};
