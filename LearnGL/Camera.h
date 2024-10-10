#ifndef CAMERA
#define CAMERA

#include "Transform.h"
#include "Utils.hpp"
#include "SystemConfig.h"

#define CAMERA_MAX_ANGLE_MULT 0.85f

struct Camera {

	transform camTrans;
	transform * target;

	float height;
	float distance;
	float pitch;
	float yaw;
	float mvSpeed;

	glm::mat4x4 viewMat;
	

	Camera(transform * T = nullptr ,float dist = 15.0f,float off = 1.0f, float spd = 3.0f);
	
	void Retarget(transform* T);

	void Y_Axis(float input = 0.0f);
	void X_Axis(float input = 0.0f);

	void Update();


};






#endif // !CAMERA

