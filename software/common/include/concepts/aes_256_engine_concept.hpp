#pragma once

#include <util/uix.hpp>
#include <system_error>
#include <concepts>
#include <span>


namespace aes_256_engine_concepts {

	using namespace ztu::uix;

	template<class T>
	concept engine_init = requires(T engine, std::span<const u8> key) {
		/**
		 * @brief Initializes the AES-256 context.
		 * 
		 * This function sets up the context for the AES-256 encryption/decryption operation.
		 * It checks if the provided key is 256-bit (32 bytes) long. If the key size is not correct,
		 * it returns an error code indicating the wrong key size (`WRONG_KEY_SIZE`). 
		 * 
		 * The key is stored internally in the context and used for all subsequent encryption/decryption operations.
		 * 
		 * @param k The 256-bit key to be used for the AES-256 operation, represented as a span of unsigned 8-bit integers.
		 * 
		 * @return std::error_code indicating the result of the operation. Zero on success, non-zero on error.
		 */
		{ engine.init(key) } -> std::same_as<std::error_code>;
	};


	template<class T>
	concept engine_encrypt = requires(T engine, 
		std::span<u8> nonce,
		std::span<u8> plainTextBuffer, usize plainTextSize,
		std::span<u8> cipherText, usize &cipherTextSize
	) {
		/**
		 * @brief Encrypts plain text using AES-256 in CBC mode with PKCS#7 padding.
		 *
		 * This function encrypts the plaintext buffer using AES-256 encryption in CBC mode. 
		 * It also adds PKCS#7 padding to the plaintext buffer. The encryption result is stored in the 
		 * ciphertext buffer.
		 * 	
		 * @param nonce				The nonce to use for encryption, must be at least `aes256::blockSize` bytes long.
		 * @param plainTextBuffer	A buffer containing the plain text to encrypt.
		 * @param cipherText		A buffer to store the encrypted cipher text, must be at least `aes256::cipherLength(plainTextSize)` bytes long.
		 * @param cipherTextSize	A variable to store the size of the encrypted cipher text.
		 *
		 * @return std::error_code indicating the result of the operation. Zero on success, non-zero on error.
		 */
		{
			engine.encrypt(
				nonce,
				plainTextBuffer, plainTextSize,
				cipherText, cipherTextSize
			)
		} -> std::same_as<std::error_code>;
	};

	template<class T>
	concept engine_decrypt = requires(T engine, 
		std::span<u8> iv,
		std::span<const u8> cipherText,
		std::span<u8> plainText, usize &plainTextSize
	) {
		/**
		 * @brief Decrypts the given `cipherText` using AES-256 algorithm in CBC mode.
		 *
		 * The `nonce` is used as the initial vector for CBC mode.
		 * The `plainText` buffer must be large enough to hold the decrypted message, otherwise
		 * `PLAIN_TEXT_BUFFER_TOO_SMALL` error will be returned. The `cipherText` size must be a multiple of the
		 * block size (16 bytes) and non-zero, otherwise `CIPHER_TEXT_BUFFER_TOO_SMALL` error will be returned.
		 * The decryption key must be set before calling this function, otherwise `USE_BEFORE_INITIALIZATION`
		 * error will be returned.
		 *
		 * The function removes the PKCS#7 padding from the decrypted message. If the padding is corrupted,
		 * the function will return `CORRUPT_PKC7_PADDING` error.
		 *
		 * @param nonce			The nonce used as the initial vector for CBC mode.
		 * @param cipherText	The buffer holding the ciphertext to be decrypted.
		 * @param plainText		The buffer to hold the decrypted plaintext.
		 * @param plainTextSize	The size of the decrypted plaintext.
		 * @return 				std::error_code indicating the result of the operation. Zero on success, non-zero on error.
		 */
		engine.decrypt(
			iv,
			cipherText,
			plainText, plainTextSize
		);
	};
}

template<class T>
concept aes_256_engine_concept = (
	aes_256_engine_concepts::engine_init<T> and
	aes_256_engine_concepts::engine_encrypt<T> and
	aes_256_engine_concepts::engine_decrypt<T>
);
