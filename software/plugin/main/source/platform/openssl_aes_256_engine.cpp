#include <platform/openssl_aes_256_engine.hpp>
#include <error_codes/aes_256_engine_error.hpp>
#include <aes_256_info.hpp>

#include <openssl/evp.h>
#include <openssl/aes.h>

#include <algorithm>


std::error_code openssl_aes_256_engine::init(std::span<const u8> newKey) {
	using aes_256_engine_error::make_error_code;
	using enum aes_256_engine_error::codes;

	if (newKey.size() != key.size()) {
		return make_error_code(WRONG_KEY_SIZE);
	}

	ctx = EVP_CIPHER_CTX_new();
	if (not ctx) {
		return make_error_code(CONTEXT_INITIALIZATION_FAILED);
	}

	std::copy(newKey.begin(), newKey.end(), key.begin());

	return make_error_code(OK);
}


std::error_code openssl_aes_256_engine::encrypt(
	std::span<u8> iv,
	std::span<u8> plainTextBuffer, const usize plainTextSize,
	std::span<u8> cipherText, usize &cipherTextSize
) const {
	using aes_256_engine_error::make_error_code;
	using enum aes_256_engine_error::codes;

	cipherTextSize = 0;

	if (not ctx)
		return make_error_code(USE_BEFORE_INITIALIZATION);

	if (iv.size() != aes_256_info::ivSize) {
		return make_error_code(INVALID_IV_SIZE);
	}

	const auto paddedSize = aes_256_info::cipherLength(plainTextSize);

	if (cipherText.size() < paddedSize)
		return make_error_code(CIPHER_TEXT_BUFFER_TOO_SMALL);

	int length, totalLength;
	if (not EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key.data(), iv.data()))
		return make_error_code(INTERNAL_ERROR);

	if (not EVP_EncryptUpdate(ctx, cipherText.data(), &length, plainTextBuffer.data(), static_cast<int>(plainTextSize)))
		return make_error_code(INTERNAL_ERROR);

	totalLength = length;

	if (not EVP_EncryptFinal_ex(ctx, cipherText.data() + length, &length))
		return make_error_code(INTERNAL_ERROR);
	totalLength += length;

	cipherTextSize = totalLength;

	return make_error_code(OK);
}


[[nodiscard]] std::error_code openssl_aes_256_engine::decrypt(
	std::span<u8> iv,
	std::span<const u8> cipherText,
	std::span<u8> plainText, usize &plainTextSize
) const {
	using aes_256_engine_error::make_error_code;
	using enum aes_256_engine_error::codes;

	plainTextSize = 0;

	if (not ctx)
		return make_error_code(USE_BEFORE_INITIALIZATION);

	if (iv.size() != aes_256_info::blockSize) {
		return make_error_code(INVALID_IV_SIZE);
	}

	int length, totalLength;

	if (not EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key.data(), iv.data()))
		return make_error_code(INTERNAL_ERROR);

	if (not EVP_DecryptUpdate(ctx, plainText.data(), &length, cipherText.data(), static_cast<int>(cipherText.size())))
		return make_error_code(INTERNAL_ERROR);

	totalLength = length;

	if (not EVP_DecryptFinal(ctx, plainText.data() + length, &length))
		return make_error_code(CORRUPT_PKC7_PADDING);

	totalLength += length;

	plainTextSize = totalLength;

	return make_error_code(OK);
}


openssl_aes_256_engine::~openssl_aes_256_engine() {
	EVP_CIPHER_CTX_free(ctx);
}
