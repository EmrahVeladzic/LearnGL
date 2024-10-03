#include "Camera.h"

Camera::Camera(transform* T, float dist, float spd ) {

	target = T;
	mvSpeed = spd / TARGET_FPS;
	distance = dist;
	pitch = 0.0f;	
	yaw = glm::pi<float>();
	camTrans.scale = glm::vec3(1.0f);
	camTrans.translation = glm::vec3(0.0f, 0.0f, -distance);
	camTrans.rotation = glm::quat(0.0f, 0.0f, 1.0f, 0.0f);
	viewMat = Utils::transToMat(camTrans);
	

	if (target != nullptr) {		

		Update();

	}

	
	
}

void Camera::Retarget(transform* T) {
	target = T;

	if (target != nullptr) {

		
		
		Update();
	
	}
}

void Camera::Y_Axis(float input) {	
	pitch += (input * mvSpeed);
	if (pitch > glm::half_pi<float>() * 0.75f)
	{
		pitch = glm::half_pi<float>() * 0.75f;
	}
	else if (pitch < -glm::half_pi<float>() * 0.75f)
	{
		pitch = -glm::half_pi<float>() * 0.75f;
	}

	Update();
		
}
void Camera::X_Axis(float input) {
	yaw += (input*mvSpeed);
	yaw = glm::mod(yaw,(glm::pi<float>()*2));
	if (yaw<0.0f)
	{
		yaw += (2 * glm::pi<float>());
	}
		
	Update();
	
}

void Camera::Update() {			
	
		
	glm::vec3 offset;
	offset.x = distance * cos(pitch) * sin(yaw); 
	offset.y = distance * sin(pitch);            
	offset.z = distance * cos(pitch) * cos(yaw); 

	
	camTrans.translation = target->translation + offset;

	
	viewMat = glm::lookAt(camTrans.translation, target->translation, glm::vec3(0.0f, 1.0f, 0.0f));


}