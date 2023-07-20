#include <domain_logic/sign.hpp>

/*
sign_storage_t sign.storage{ };
sign_animation_controller_t sign.animation_controller{ };
std::atomic_flag sign.switch_task{ ATOMIC_FLAG_INIT };
*/

sign_t sign{
	.storage{ },
	.animation_controller{ },
	.switch_task{ ATOMIC_FLAG_INIT }
};
