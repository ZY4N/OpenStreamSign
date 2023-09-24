#include <domain_logic/sign.hpp>

sign_t sign{
	.storage{ },
	.animation_controller{ },
	.switch_task{ ATOMIC_FLAG_INIT }
};
