#pragma once

#include <concepts/aes_256_engine_concept.hpp>
#include <platform/mbedtls_aes_256_engine.hpp>

static_assert(aes_256_engine_concept<mbedtls_aes_256_engine>);

using aes_256_engine = mbedtls_aes_256_engine;
