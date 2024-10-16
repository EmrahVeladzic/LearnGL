#ifndef AI_HANDLER
#define AI_HANDLER

#include "Actor.hpp"

namespace AI {

	size_t Move(Actor * executor ,float angle, float yaw = 0.0f);

	size_t Move(Actor* executor, empty target, bool inherit = false);

	size_t Move(Actor* executor);

}



#endif // !AI_HANDLER
