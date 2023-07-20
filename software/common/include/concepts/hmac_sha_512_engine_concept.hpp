#pragma once

#include <util/uix.hpp>
#include <concepts>
#include <system_error>
#include <span>

namespace hmac_sha_512_engine_concepts {
	using namespace ztu::uix;

	template<class T>
	concept engine_init = requires(T engine, std::span<const u8> key) {
		/**
		 * @brief Initializes the HMAC-SHA512 context.
		 * 
		 * This function sets up the context for the HMAC SHA512 calculation.
		 * Returns an error if the provided key is not 64 bytes long.
		 *
		 * @param k The 512-bit key to be used for the HMAC-SHA512 operation, represented as a span of unsigned 8-bit integers.
		 *
		 * @return An 'std::error_code' that indicates the status of the function. Returns '0' on success and a non-zero value on error.
		 */
		{ engine.init(key) } -> std::same_as<std::error_code>;
	};


	template<class T>
	concept engine_hash = requires(T engine, 
		std::span<const u8> src,
		std::span<u8> dst
	) {
		/**
		 * Computes the hash of the input 'src' using SHA-512 and the already provided key.
		 * 
		 * @param src The input data to be hashed.
		 * @param dst The output buffer where the HMAC will be stored.
		 * 
		 * @return An 'std::error_code' that indicates the status of the function. Returns '0' on success and a non-zero value on error.
		 * 
		 */
		{ engine.hash(src, dst) } -> std::same_as<std::error_code>;
	};
}


template<class T>
concept hmac_sha_512_engine_concept = (
	hmac_sha_512_engine_concepts::engine_init<T> and
	hmac_sha_512_engine_concepts::engine_hash<T>
);
