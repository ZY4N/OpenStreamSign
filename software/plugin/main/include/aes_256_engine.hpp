#pragma once

#include <concepts/aes_256_engine_concept.hpp>
#include <platform/openssl_aes_256_engine.hpp>

static_assert(aes_256_engine_concept<openssl_aes_256_engine>);

using aes_256_engine = openssl_aes_256_engine;
