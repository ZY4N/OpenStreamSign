#pragma once

#include <openssl/hmac.h>
#include <util/uix.hpp>
#include <system_error>
#include <array>
#include <span>

using namespace ztu::uix;

class openssl_hmac_sha_512_engine {
public:
	[[nodiscard]] std::error_code init(std::span<const u8> key);

	[[nodiscard]] std::error_code hash(
		std::span<const u8> src,
		std::span<u8> dst
	);

	[[nodiscard]] bool initialized() const;

	~openssl_hmac_sha_512_engine();

private:
	EVP_MAC *mac;
	EVP_MAC_CTX *ctx;
	std::array<u8, 64> key;
};
