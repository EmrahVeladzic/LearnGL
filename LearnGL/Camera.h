#ifndef CAMERA
#define CAMERA

#include "Transform.h"
#include "Utils.hpp"

struct Camera {


	transform camT;

	float camD;

	float camO;
	float camVO;
	


	Camera();

	void Setup(transform mT);

	void Yaw(transform mT, float value);

	void Pitch(transform mT, float value);

	void Update(transform mT);

};






#endif // !CAMERA

