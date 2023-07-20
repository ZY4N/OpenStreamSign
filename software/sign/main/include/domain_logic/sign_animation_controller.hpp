#pragma once

#include <platform/animation_handler.hpp>
#include <domain_logic/sign_animation.hpp>
#include <domain_logic/sign_state.hpp>

using sign_animation_handler_t = animation_handler<sign_animations>;

class sign_animation_controller_t {
public:
	void init(sign_state initialState);

	void setState(sign_state newState);

	void setAnimation(sign_state state, const sign_animation& newAnimation);

private:
	void updateAnimation();

	sign_animation_handler_t animationHandler;
	std::array<sign_animation, static_cast<size_t>(sign_state::LAST)> animations;
	sign_state currentState;
};
