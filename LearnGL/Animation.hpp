#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <vector>
#include <iostream>


struct transform {

	glm::vec3 translation = glm::vec3(0.0f, 0.0f, 0.0f);

	glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);

	glm::quat rotation = glm::quat(0.0f, 0.0f, 1.0f, 0.0f);


};


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
};


