#pragma once

#include <array>
#include <span>
#include <cinttypes>

namespace ztu::base64 {
	using size_t = std::size_t;
	using ssize_t = std::make_signed_t<size_t>;

	constexpr std::array<char, 64> lookup {
		'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
		'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
		'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
		'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
		'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
		'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
		'w', 'x', 'y', 'z', '0', '1', '2', '3',
		'4', '5', '6', '7', '8', '9', '+', '/'
	};

	constexpr std::array<uint8_t, 80> invLookup {
		62, 0, 0, 0, 63, 52, 53, 54, 55, 56,
		57, 58, 59, 60, 61, 0, 0, 0, 0, 0,
		0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
		10, 11, 12, 13, 14, 15, 16, 17, 18,
		19, 20, 21, 22, 23, 24, 25, 0, 0, 0,
		0, 0, 0, 26, 27, 28, 29, 30, 31, 32,
		33, 34, 35, 36, 37, 38, 39, 40, 41,
		42, 43, 44, 45, 46, 47, 48, 49, 50, 51
	};

	constexpr auto invOffset = 43;


	constexpr size_t encodedSize(size_t byteSize) {
		return (byteSize * 4 + 2) / 3; // integer ceil
	}

	constexpr size_t decodedSize(size_t encodedSize) {
		return (encodedSize * 3) / 4; // integer floor
	}


	constexpr bool encode(
		std::span<const uint8_t> src, std::span<char> dst
	) {
		if (dst.size() < encodedSize(src.size()))
			return false;

		ssize_t srcIndex = 0, dstIndex = 0;

		const auto process = [&]<ssize_t Chars>(uint32_t input) {
			for (ssize_t k = 0; k < Chars; k++) {
				dst[dstIndex++] = lookup[(input >> (18 - k * 6)) & 0x3f];
			}
		};

		const auto srcSize = static_cast<ssize_t>(src.size());
		while (srcIndex + 3 <= srcSize) {
			process.template operator()<4>(
				(src[srcIndex + 0] << 16) |
				(src[srcIndex + 1] << 8) |
				(src[srcIndex + 2] << 0)
			);
			srcIndex += 3;
		}

		const auto bytesLeft = src.size() % 3;

		if (bytesLeft == 1) {
			process.template operator()<2>(
				(src[srcIndex + 0] << 16)
			);
		} else if (bytesLeft == 2) {
			process.template operator()<3>(
				(src[srcIndex + 0] << 16) |
				(src[srcIndex + 1] << 8)
			);
		}

		return true;
	}

	constexpr bool decode(
		std::span<const char> src, std::span<uint8_t> dst
	) {
		if (dst.size() < decodedSize(src.size()))
			return false;

		ssize_t srcIndex = 0, dstIndex = 0;

		const auto process = [&]<ssize_t NumBytes>(auto &output) {
			output = 0;
			constexpr ssize_t invLookupSize = static_cast<ssize_t>(invLookup.size());
			for (ssize_t k = 0; k < NumBytes; k++) {
				output <<= 6;
				const auto index = src[srcIndex++] - invOffset;
				if (index < invLookupSize) {
					output |= invLookup[index];
				} else {
					return false;
				}
			}
			return true;
		};

		const auto srcSize = static_cast<ssize_t>(src.size());
		while (srcIndex + 4 <= srcSize) {
			uint32_t next24Bits;
			if (not process.template operator()<4>(next24Bits)) return false;
 			dst[dstIndex + 0] = uint8_t(next24Bits >> 16);
			dst[dstIndex + 1] = uint8_t(next24Bits >> 8);
			dst[dstIndex + 2] = uint8_t(next24Bits >> 0);
			dstIndex += 3;
		}

		const auto bytesLeft = dst.size() % 3;

		if (bytesLeft == 1) {
			uint16_t next12Bits;
			if (not process.template operator()<2>(next12Bits)) return false;
			dst[dstIndex + 0] = uint8_t(next12Bits >> 4);
		} else if (bytesLeft == 2) {
			uint32_t next18Bits;
			if (not process.template operator()<3>(next18Bits)) return false;
			dst[dstIndex + 0] = uint8_t(next18Bits >> 10);
			dst[dstIndex + 1] = uint8_t(next18Bits >> 2);
		}

		return true;
	}
}
