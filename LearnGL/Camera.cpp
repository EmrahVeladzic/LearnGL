#include "Camera.h"

Camera::Camera()
{
	camD = -15.0f;
	camO = glm::pi<float>();
	camVO = 0.0f;

}

void Camera::Setup(transform mT) {


	camT.translation = mT.translation + glm::vec3(0.0f, camVO, camD);
	camT.rotation = glm::rotate(glm::mat4x4(1.0f), camO, glm::vec3(0.0, 1.0, 0.0));
	camT.scale = glm::vec3(1.0f, 1.0f, 1.0f);

}

void Camera::Yaw(transform mT, float value) {

	camO += value;
	camO = glm::mod(camO, (glm::pi<float>() * 2));

	Update(mT);

}

void Camera::Pitch(transform mT, float value) {

	camVO += value;

	if (camVO > glm::half_pi<float>()) {
		camVO = glm::half_pi<float>();
	}

	else if (camVO < -glm::half_pi<float>()) {
		camVO = -glm::half_pi<float>();
	}

	Update(mT);

}

void Camera::Update(transform mT) {

	camT.translation = mT.translation + glm::vec3(0.0f, 0.0f, camD);

	glm::mat4x4 camMat = glm::mat4x4(1.0f);

	camMat = glm::translate(camMat, mT.translation);

	camT.rotation = glm::rotate(camMat, camO, glm::vec3(0.0f, 1.0f, 0.0f));

	camMat = Utils::transToMat(camT);

	glm::quat localX = glm::angleAxis(camO, glm::vec3(0.0f, 1.0f, 0.0f));


	camT.rotation = glm::rotate(camMat, camVO, glm::vec3(1.0f, 0.0f, 0.0f) * localX);

}