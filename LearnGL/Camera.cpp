#include "Camera.h"

Camera::Camera(transform* T, float dist,float off, float spd ) {

	target = T;
	mvSpeed = spd / TARGET_FPS;
	distance = dist;
	height = off;
	pitch = 0.5f;	
	yaw = glm::pi<float>();	
	camTrans.translation = glm::vec3(0.0f,0.0f, -distance);
	
	viewMat = camTrans.Matrix();
	

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
	if (pitch > glm::half_pi<float>() * CAMERA_MAX_ANGLE_MULT)
	{
		pitch = glm::half_pi<float>() * CAMERA_MAX_ANGLE_MULT;
	}
	else if (pitch < -glm::half_pi<float>()* CAMERA_MAX_ANGLE_MULT)
	{
		pitch = -glm::half_pi<float>() * CAMERA_MAX_ANGLE_MULT;
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

	
	viewMat = glm::lookAt(camTrans.translation, target->translation+ glm::vec3(0.0f,height,0.0f), glm::vec3(0.0f, 1.0f, 0.0f));


}