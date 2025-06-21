#include "AI_Handler.hpp"



size_t AI::Move(Actor * executor) {

	glm::vec3 nDiff = glm::normalize(executor->AI_Node.target.translation - executor->Trans.translation);

	if (glm::distance(executor->AI_Node.target.translation, executor->Trans.translation) > executor->AI_Node.targetDistance)
	{
		glm::mat4x4 look = glm::lookAt(executor->AI_Node.target.translation, executor->Trans.translation, glm::vec3(0.0f, 1.0f, 0.0f));

		executor->Trans.rotation = glm::slerp(executor->Trans.rotation, transform(look).rotation, executor->mvSpeed);

		executor->Trans.translation += nDiff * executor->mvSpeed;


		if (!executor->Model->bones.empty() && executor->Model->bones[0].animations.size() > 1) {
			executor->Model->currentAnim = 1;
		}

		return 0;
	}


	else
	{
		if (!executor->Model->bones.empty() && !executor->Model->bones[0].animations.empty()) {
			executor->Model->currentAnim = 0;
		}

		return 1;

	}


	
}


size_t AI::Move(Actor * executor,empty target , bool inherit) {

	glm::vec3 nDiff = glm::normalize(target.translation-executor->Trans.translation);
	float angle =2.0f * glm::acos( glm::clamp(glm::dot(executor->Trans.rotation, target.rotation), -1.0f, 1.0f));

	if (glm::distance(target.translation,executor->Trans.translation)>executor->AI_Node.targetDistance)
	{
		glm::mat4x4 look = glm::lookAt(target.translation, executor->Trans.translation, glm::vec3(0.0f, 1.0f, 0.0f));

		executor->Trans.rotation = glm::slerp(executor->Trans.rotation, transform(look).rotation,executor->mvSpeed);

		executor->Trans.translation += nDiff * executor->mvSpeed;
		

		if (!executor->Model->bones.empty() && executor->Model->bones[0].animations.size() > 1) {
			executor->Model->currentAnim = 1;
		}

		return 0;
	}

	
	else if (glm::abs(angle)>0.0f && inherit)
	{
		executor->Trans.rotation = glm::slerp(executor->Trans.rotation, target.rotation, executor->mvSpeed);
		

		if (!executor->Model->bones.empty() && executor->Model->bones[0].animations.size() > 1) {
				executor->Model->currentAnim = 1;
		}

		return 0;
	}



	else
	{
		if (!executor->Model->bones.empty() && !executor->Model->bones[0].animations.empty()) {
			executor->Model->currentAnim = 0;
		}

		return 1;
		
	}

	
	
}


size_t AI::Move(Actor * executor, float Angle, float Y) {

	if (executor->Cam != nullptr) {
		Y = executor->Cam->yaw;
	}

	executor->AI_Node.target.rotation = glm::angleAxis(Angle + Y + glm::pi<float>(), glm::vec3(0.0f, 1.0f, 0.0f));

	executor->Trans.rotation = glm::slerp(executor->Trans.rotation, executor->AI_Node.target.rotation, executor->mvSpeed);

	executor->Trans.translation += executor->Trans.rotation * glm::vec3(0.0f, 0.0f, executor->mvSpeed);

	if (executor->Cam != nullptr) {
		executor->Cam->Update();
	}

	if (!executor->Model->bones.empty() && executor->Model->bones[0].animations.size() > 1) {
		executor->Model->currentAnim = 1;
	}


	return 0;
}