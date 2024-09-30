#ifndef TRANSFORM
#define TRANSFORM

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

struct transform {

	glm::vec3 translation;

	glm::vec3 scale;

	glm::quat rotation;


	transform(glm::vec3 t = glm::vec3(0.0f, 0.0f, 0.0f), glm::quat r = glm::quat(0.0f, 0.0f, 1.0f, 0.0f), glm::vec3 s = glm::vec3(1.0f, 1.0f, 1.0f))
	{
		translation = t;

		rotation = r;

		scale = s;

	}

};


#endif // !TRANSFORM
