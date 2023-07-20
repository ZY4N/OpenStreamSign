#pragma once

#include <variant>

#include "temporal_scalers/ping_pong_scaler.hpp"
#include "temporal_scalers/sinus_scaler.hpp"
#include "temporal_scalers/random_scaler.hpp"
#include "temporal_scalers/smooth_random_scaler.hpp"


template<class temporal_scalers_t>
using temporal_scaler = std::variant<
	typename temporal_scalers_t::ping_pong,
	typename temporal_scalers_t::sinus,
	typename temporal_scalers_t::random,
	typename temporal_scalers_t::smooth_random
>;


template<u32 TicksPerColor>
struct temporal_scalers {
	using ping_pong = temporal_scalers_detail::ping_pong_scaler<TicksPerColor>;
	using sinus = temporal_scalers_detail::sinus_scaler<TicksPerColor>;
	using random = temporal_scalers_detail::random_scaler<TicksPerColor>;
	using smooth_random = temporal_scalers_detail::smooth_random_scaler<TicksPerColor>;
};
