#pragma once

#include <concepts/hmac_sha_512_engine_concept.hpp>
#include <platform/mbedtls_hmac_sha_512_engine.hpp>

static_assert(hmac_sha_512_engine_concept<mbedtls_hmac_sha_512_engine>);

using hmac_sha_512_engine = mbedtls_hmac_sha_512_engine;
