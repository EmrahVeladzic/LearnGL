#ifndef ANIMATION
#define ANIMATION

#include "Transform.h"
#include <glm/gtx/matrix_decompose.hpp>
#include <vector>
#include <iostream>





struct animJoint {

	std::vector<glm::vec3> translations;

	std::vector<glm::quat> rotations;

	std::vector<glm::vec3> scales;

	std::vector<float> transTimes;

	std::vector<float> rotTimes;

	std::vector<float> scalTimes;

	int transIndex;
	int rotIndex;
	int scalIndex;

	float transInterpolation;
	float rotInterpolation;
	float scalInterpolation;

	int Tbegin_index;
	int Tend_index;

	int Rbegin_index;
	int Rend_index;

	int Sbegin_index;
	int Send_index;

	animJoint()
	{
		transIndex = 0;
		rotIndex = 0;
		scalIndex = 0;
		transInterpolation = 0.0f;
		rotInterpolation = 0.0f;
		scalInterpolation = 0.0f;
		Tbegin_index = 0;
		Tend_index = 1;
		Rbegin_index = 0;
		Rend_index = 1;
		Sbegin_index = 0;
		Send_index = 1;
	}
};


#endif // !ANIMATION
