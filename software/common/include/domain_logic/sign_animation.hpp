#pragma once

#include <lighting/animation.hpp>

constexpr auto maxSequenceLength = 8;
constexpr auto ticksPerColor = 1024;
constexpr auto numPixels = 16;
constexpr auto numFrames = 8;

using sign_suppliers = color_suppliers<maxSequenceLength>;
using sign_supplier = color_supplier<sign_suppliers>;

using sign_scalers = temporal_scalers<ticksPerColor>;
using sign_scaler = temporal_scaler<sign_scalers>;

using sign_sequencer = color_sequencer<ticksPerColor, sign_supplier>;

using sign_mix_type = color_mixing::type;

using sign_animations = animations<ticksPerColor, numPixels, numFrames, sign_sequencer, sign_scaler>;
using sign_animation = variable_speed_animation<sign_animations>;
