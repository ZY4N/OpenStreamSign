#pragma once

#include "mbedtls/md.h"

#include <util/uix.hpp>
#include <system_error>
#include <array>
#include <span>

using namespace ztu::uix;

/**
 * @class hmacSHA512
 * 
 * @brief This class provides a wrapper for the mbedtls implementation of the HMAC-SHA512 algorithm.
 * 
 * The class provides a simple interface for initializing the context, computing the HMAC-SHA512 hash of an input message,
 * and freeing the resources used by the context.
 * 
 * The implementation uses mbedtls for the underlying hash calculations.
 */
class mbedtls_hmac_sha_512_engine {
public:
	mbedtls_hmac_sha_512_engine() = default;


	/**
	 * @brief Initializes the HMAC-SHA512 context.
	 * 
	 * This function sets up the context for the HMAC SHA512 calculation.
	 * It checks if the provided key is 64 bytes long. If the key size is not correct,
	 * it returns an error code indicating the wrong key size. 
 	 * 
 	 * If the context does not yet exist, it initializes the internal mbedtls context with SHA512
	 * as the digest algorithm.
	 *
	 * @param k The 512-bit key to be used for the HMAC-SHA512 operation, represented as a span of unsigned 8-bit integers.
	 *
	 * @return An 'std::error_code' that indicates the status of the function. Returns '0' on success and a non-zero value on error.
	 */
	[[nodiscard]] std::error_code init(std::span<const uint8_t> key);


	/**
	 * Computes the hash of the input `src` using SHA-512.
	 * 
	 * @param src The input data to be hashed.
	 * @param dst The output buffer where the HMAC will be stored.
	 * 
	 * @return An 'std::error_code' that indicates the status of the function. Returns '0' on success and a non-zero value on error.
	 * 
	 */
	[[nodiscard]] std::error_code hash(
		std::span<const uint8_t> src,
		std::span<uint8_t> dst
	);

	~mbedtls_hmac_sha_512_engine();

private:
	mbedtls_md_context_t ctx;
	std::array<u8, 64> key;
	bool initialized{ false }; // necessary as access to ctx are private
};
