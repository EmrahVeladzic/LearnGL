#ifndef CAMERA
#define CAMERA

#include "Transform.h"
#include "Utils.hpp"
#include "SystemConfig.h"

struct Camera {

	transform camTrans;
	transform * target;

	float distance;
	float pitch;
	float yaw;
	float mvSpeed;

	glm::mat4x4 viewMat;
	

	Camera(transform * T = nullptr ,float dist = 7.5f, float spd = 1.0f);
	
	void Retarget(transform* T);

	void Y_Axis(float input = 0.0f);
	void X_Axis(float input = 0.0f);

	void Update();


};






#endif // !CAMERA

