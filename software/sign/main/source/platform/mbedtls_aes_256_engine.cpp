#include <platform/mbedtls_aes_256_engine.hpp>
#include <error_codes/aes_256_engine_error.hpp>

#include <algorithm>

#include <esp_log.h>


mbedtls_aes_256_engine::mbedtls_aes_256_engine(const mbedtls_aes_256_engine& other) {
	init(other.key);
}

mbedtls_aes_256_engine::mbedtls_aes_256_engine(mbedtls_aes_256_engine&& other) {
	ctx = other.ctx;
	key = other.key;
	std::swap(initialized, other.initialized);
}

mbedtls_aes_256_engine &mbedtls_aes_256_engine::operator=(const mbedtls_aes_256_engine& other) {
	this->~mbedtls_aes_256_engine();
	init(other.key);
	return *this;
}

mbedtls_aes_256_engine &mbedtls_aes_256_engine::operator=(mbedtls_aes_256_engine&& other) {
	if (&other != this) {
		this->~mbedtls_aes_256_engine();
		ctx = other.ctx;
		key = other.key;
		std::swap(initialized, other.initialized);
	}
	return *this;
}

std::error_code mbedtls_aes_256_engine::init(std::span<const u8> newKey) {
	using aes_256_engine_error::make_error_code;
	using enum aes_256_engine_error::codes;

	if (newKey.size() != key.size()) {
		return make_error_code(WRONG_KEY_SIZE);
	}

	std::copy(newKey.begin(), newKey.end(), key.begin());

	initialized = true;

	return make_error_code(OK);
}

[[nodiscard]] std::error_code mbedtls_aes_256_engine::encrypt(
	std::span<u8> iv,
	std::span<u8> plainTextBuffer, usize plainTextSize,
	std::span<u8> cipherText, usize &cipherTextSize
) {
	using aes_256_engine_error::make_error_code;
	using enum aes_256_engine_error::codes;

	cipherTextSize = 0; // in case error code is ignored

	if (not initialized) {
		return make_error_code(USE_BEFORE_INITIALIZATION);
	}

	if (iv.size() != aes_256_info::blockSize) {
		return make_error_code(INVALID_IV_SIZE);
	}

	const auto paddedSize = aes_256_info::cipherLength(plainTextSize);

	if (plainTextBuffer.size() < paddedSize) {
		return make_error_code(PLAIN_TEXT_BUFFER_TOO_SMALL);
	}

	if (cipherText.size() < paddedSize) {
		return make_error_code(CIPHER_TEXT_BUFFER_TOO_SMALL);
	}

	// add PKCS#7 padding
	const auto pkc7 = uint8_t(paddedSize - plainTextSize);
	std::fill(plainTextBuffer.begin() + plainTextSize, plainTextBuffer.begin() + paddedSize, pkc7);

	mbedtls_aes_init(&ctx);

	// can only throw MBEDTLS_ERR_AES_INVALID_KEY_LENGTH
	mbedtls_aes_setkey_enc(&ctx, key.data(), 256);

	// can only throw MBEDTLS_ERR_AES_INVALID_INPUT_LENGTH which was checked before
	mbedtls_aes_crypt_cbc(
		&ctx, MBEDTLS_AES_ENCRYPT,
		paddedSize,
		iv.data(),
		plainTextBuffer.data(),
		cipherText.data()
	);

	cipherTextSize = paddedSize;
	
	return make_error_code(OK);
}

[[nodiscard]] std::error_code mbedtls_aes_256_engine::decrypt(
	std::span<u8> iv,
	std::span<const u8> cipherText,
	std::span<u8> plainText, usize &plainTextSize
) {
	using aes_256_engine_error::make_error_code;
	using enum aes_256_engine_error::codes;

	plainTextSize = 0;

	if (not initialized) {
		return make_error_code(USE_BEFORE_INITIALIZATION);
	}

	if (iv.size() != aes_256_info::blockSize) {
		return make_error_code(INVALID_IV_SIZE);
	}

	if (cipherText.size() == 0 || cipherText.size() % aes_256_info::blockSize != 0) {
		return make_error_code(CIPHER_TEXT_BUFFER_TOO_SMALL);
	}

	if (plainText.size() < cipherText.size()) {
		return make_error_code(PLAIN_TEXT_BUFFER_TOO_SMALL);
	}

	mbedtls_aes_init(&ctx);

	mbedtls_aes_setkey_dec(&ctx, key.data(), key.size() * 8);

	mbedtls_aes_crypt_cbc(
		&ctx, MBEDTLS_AES_DECRYPT,
		cipherText.size(),
		iv.data(),
		cipherText.data(),
		plainText.data()
	);

	const auto pkc7PaddingSize = plainText[cipherText.size() - 1];

	if (pkc7PaddingSize == 0 or pkc7PaddingSize > aes_256_info::blockSize) {
		return make_error_code(CORRUPT_PKC7_PADDING);
	}

	plainTextSize = cipherText.size() - pkc7PaddingSize;
	
	return make_error_code(OK);
}

mbedtls_aes_256_engine::~mbedtls_aes_256_engine() {
	if (initialized) {
		mbedtls_aes_free(&ctx);
	}
}
