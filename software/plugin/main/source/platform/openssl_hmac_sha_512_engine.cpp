#include <platform/openssl_hmac_sha_512_engine.hpp>
#include <error_codes/hmac_sha_512_engine_error.hpp>
#include <algorithm>


std::error_code openssl_hmac_sha_512_engine::init(const std::span<const u8> newKey) {
	using hmac_sha_512_engine_error::make_error_code;
	using enum hmac_sha_512_engine_error::codes;

	if (newKey.size() != key.size()) {
		return make_error_code(WRONG_KEY_SIZE);
	}

	mac = EVP_MAC_fetch(nullptr, "HMAC", nullptr);
	if (not mac) {
		return make_error_code(CONTEXT_INITIALIZATION_FAILED);
	}

	ctx = EVP_MAC_CTX_new(mac);
	if (not ctx) {
		EVP_MAC_free(mac);
		return make_error_code(CONTEXT_INITIALIZATION_FAILED);
	}

	auto algorithm = EVP_MD_get0_name(EVP_sha512());

	OSSL_PARAM subalg_param[] = {
		OSSL_PARAM_construct_utf8_string("digest", (char*)algorithm, 0), // cast const away ¯\_(ツ)_/¯
		OSSL_PARAM_END
	};

	EVP_MAC_CTX_set_params(ctx, subalg_param);

	std::copy(newKey.begin(), newKey.end(), key.begin());

	return make_error_code(OK);
}


std::error_code openssl_hmac_sha_512_engine::hash(
	std::span<const u8> src,
	std::span<u8> dst
) {
	using hmac_sha_512_engine_error::make_error_code;
	using enum hmac_sha_512_engine_error::codes;

	if (not ctx) {
		return make_error_code(USE_BEFORE_INITIALIZATION);
	}

	if (not EVP_MAC_init(ctx, key.data(), key.size(), nullptr)) {
		EVP_MAC_free(mac);
		return make_error_code(INTERNAL_ERROR);
	}

	if (not EVP_MAC_update(ctx, src.data(), src.size()))
		return make_error_code(INTERNAL_ERROR);

	if (not EVP_MAC_final(ctx, dst.data(), nullptr, dst.size())) {
		return make_error_code(INTERNAL_ERROR);
	}

	return make_error_code(OK);
}


openssl_hmac_sha_512_engine::~openssl_hmac_sha_512_engine() {
	EVP_MAC_CTX_free(ctx);
	EVP_MAC_free(mac);
}
