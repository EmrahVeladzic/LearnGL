#ifndef MATERIALS
#define MATERIALS

#include <glm/glm.hpp>

#endif // !MATERIALS



class Material {

private:

	glm::vec4 ambient;
	glm::vec4 diffuse;
	glm::vec4 specular;

	float shininess;
	float transparency;
	float emmision;


public:

	float vec4_export[4];

	Material(glm::vec4 ambi, glm::vec4 diff, glm::vec4 spec, float shin, float trans, float emmis);
	glm::vec4 get_ambient();
	glm::vec4 get_diffuse();
	glm::vec4 get_specular();

	float* get_ambient_float();
	float* get_diffuse_float();
	float* get_specular_float();

	float get_shininess();
	float get_transparency();
	float get_emmision();

};

