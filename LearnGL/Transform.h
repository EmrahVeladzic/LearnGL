#ifndef TRANSFORM
#define TRANSFORM

#include "GL_Math.h"

struct empty
{
	glm::vec3 translation;

	glm::quat rotation;

	empty(glm::vec3 t = glm::vec3(0.0f), glm::quat r = glm::quat(1.0f, glm::vec3(0.0f)));

};

struct transform : empty{
	
	glm::vec3 scale;	

	transform(glm::vec3 t = glm::vec3(0.0f), glm::quat r = glm::quat(1.0f, glm::vec3(0.0f)), glm::vec3 s = glm::vec3(1.0f));

	transform(glm::mat4x4 matrix);

	glm::mat4x4 Matrix();

};


#endif // !TRANSFORM
