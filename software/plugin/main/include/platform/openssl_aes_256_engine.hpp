#pragma once

#include <openssl/evp.h>
#include <util/uix.hpp>
#include <system_error>
#include <array>
#include <span>

using namespace ztu::uix;

class openssl_aes_256_engine {
public:
	openssl_aes_256_engine() = default;

	[[nodiscard]] std::error_code init(std::span<const u8> key);
	
	[[nodiscard]] std::error_code encrypt(
		std::span<u8> nonce,
		std::span<u8> plainText, usize plainTextSize,
		std::span<u8> cipherText, usize &cipherTextSize
	) const;

	[[nodiscard]] std::error_code decrypt(
		std::span<u8> nonce,
		std::span<const u8> cipherText,
		std::span<u8> plainText, usize &plainTextSize
	) const;

	~openssl_aes_256_engine();

private:
	EVP_CIPHER_CTX* ctx{ nullptr };
	std::array<u8, 32> key{};
};
