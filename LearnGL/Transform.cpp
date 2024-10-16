#include "Transform.h"

empty::empty(glm::vec3 t, glm::quat r) {

	translation = t;
	rotation = r;
}


transform::transform(glm::vec3 t, glm::quat r, glm::vec3 s):empty(t,r){

	
	scale = s;

}

transform::transform(glm::mat4x4 matrix) {
		
		
		glm::vec3 skew;
		glm::vec4 perspective;

		glm::decompose(matrix, scale, rotation, translation, skew, perspective);

		rotation = glm::normalize(glm::conjugate(rotation));
		
}


glm::mat4x4 transform::Matrix() {


	glm::vec3 Vx = rotation * glm::vec3(1.0f, 0.0f, 0.0f);
	glm::vec3 Vy = rotation * glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 Vz = rotation * glm::vec3(0.0f, 0.0f, 1.0f);

	Vx = Vx * scale.x;
	Vy = Vy * scale.y;
	Vz = Vz * scale.z;

	glm::vec3 Vp = translation;


	return glm::mat4x4(

		Vx.x, Vx.y, Vx.z, 0.0f,
		Vy.x, Vy.y, Vy.z, 0.0f,
		Vz.x, Vz.y, Vz.z, 0.0f,
		Vp.x, Vp.y, Vp.z, 1.0f

	);
}