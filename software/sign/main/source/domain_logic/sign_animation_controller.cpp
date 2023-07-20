#include <domain_logic/sign_animation_controller.hpp>
#include <domain_logic/sign.hpp>

void sign_animation_controller_t::init(sign_state initialState) {

	animations[static_cast<size_t>(sign_state::IDLE)] = sign.storage.get<storage_keys::IDLE_ANIMATION>();
	animations[static_cast<size_t>(sign_state::SETUP)] = sign.storage.get<storage_keys::SETUP_ANIMATION>();

	animationHandler.init(animations[static_cast<size_t>(initialState)]);
}

void sign_animation_controller_t::setState(sign_state newState) {
	currentState = newState;
	updateAnimation();
}

void sign_animation_controller_t::setAnimation(sign_state state, const sign_animation& newAnimation)  {
	const auto stateIndex = static_cast<size_t>(state);
	assert(stateIndex < animations.size());
	animations[stateIndex] = newAnimation;
	if (state == currentState) {
		updateAnimation();
	}
	if (state == sign_state::IDLE) {
		sign.storage.set<storage_keys::IDLE_ANIMATION>(newAnimation);
		sign.storage.save();
	} else if (state == sign_state::SETUP) {
		sign.storage.set<storage_keys::SETUP_ANIMATION>(newAnimation);
		sign.storage.save();
	}
}

void sign_animation_controller_t::updateAnimation() {
	animationHandler.setAnimation(animations[static_cast<size_t>(currentState)]);
}
