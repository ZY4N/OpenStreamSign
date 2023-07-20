#pragma once

#include <concepts/hmac_sha_512_engine_concept.hpp>
#include <platform/openssl_hmac_sha_512_engine.hpp>

static_assert(hmac_sha_512_engine_concept<openssl_hmac_sha_512_engine>);

using hmac_sha_512_engine = openssl_hmac_sha_512_engine;
