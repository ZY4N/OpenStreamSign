#include <platform/mbedtls_hmac_sha_512_engine.hpp>
#include <error_codes/hmac_sha_512_engine_error.hpp>

#include <algorithm>

std::error_code mbedtls_hmac_sha_512_engine::init(const std::span<const u8> newKey) {
	using hmac_sha_512_engine_error::make_error_code;
	using enum hmac_sha_512_engine_error::codes;

	if (newKey.size() != key.size()) {
		return make_error_code(WRONG_KEY_SIZE);
	}
	
	std::copy(newKey.begin(), newKey.end(), key.begin());

	if (not initialized) {
		mbedtls_md_init(&ctx);

		if (mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA512), 1) != 0) {
			return make_error_code(CONTEXT_INITIALIZATION_FAILED);
		}

		initialized = true;
	}
	
	return make_error_code(OK);
}

std::error_code mbedtls_hmac_sha_512_engine::hash(
	std::span<const u8> src,
	std::span<u8> dst
) {
	using hmac_sha_512_engine_error::make_error_code;
	using enum hmac_sha_512_engine_error::codes;

	if (not initialized) {
		return make_error_code(USE_BEFORE_INITIALIZATION);
	}

	if (dst.size() < 64) {
		return make_error_code(WRONG_HASH_SIZE);
	}

	int e;
	if ((e = mbedtls_md_hmac_starts(&ctx, key.data(), key.size()))) {
		return make_error_code(INTERNAL_ERROR);
	}

	if ((e = mbedtls_md_hmac_update(&ctx, src.data(), src.size()))) {
		if (e == MBEDTLS_ERR_MD_BAD_INPUT_DATA) {
			return make_error_code(WRONG_MESSAGE_SIZE);
		} else {
			return make_error_code(INTERNAL_ERROR);
		}
	}

	if ((e = mbedtls_md_hmac_finish(&ctx, dst.data()))) {
		return make_error_code(INTERNAL_ERROR);
	}

	return make_error_code(OK);
}

mbedtls_hmac_sha_512_engine::~mbedtls_hmac_sha_512_engine() {
	mbedtls_md_free(&ctx);
}
