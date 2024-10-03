#ifndef LIGHTS
#define LIGHTS

#include <iostream>
#include <glm/glm.hpp>







class LightGeneric
{
protected:

	glm::vec4 ambient;
	glm::vec4 diffuse;
	glm::vec4 specular;
	
	glm::mat4 light_matrix;

public:

	float vec4_export[4];
	float vec3_export[3];

	LightGeneric(glm::vec4 ambi, glm::vec4 diff, glm::vec4 spec);
	bool enabled;
	glm::vec4 get_ambient();
	glm::vec4 get_diffuse();
	glm::vec4 get_specular();

	float* get_ambient_float();
	float* get_diffuse_float();
	float* get_specular_float();
	
};

class LightDirectional: virtual public LightGeneric {

protected:

	glm::vec3 direction;

public:

	void set_direction(glm::vec3 dir);
	LightDirectional(glm::vec4 ambi, glm::vec4 diff, glm::vec4 spec, glm::vec3 dir);
	glm::vec3 get_direction();
	float* get_direction_float();

};

class LightPositional : virtual public LightGeneric {

protected:


	glm::vec3 position;

	float constant;
	float linear;
	float quadratic;

public:

	float attenuation;
	


	void set_position(glm::vec3 pos);
	LightPositional(glm::vec4 ambi, glm::vec4 diff, glm::vec4 spec, glm::vec3 pos , float con, float lin, float quad);
	float calculate_attenuation(float con, float lin, float quad);
	glm::vec3 get_position();
	float* get_position_float();

};

class LightSpot : public LightDirectional, public LightPositional{


private:

	float cutoff;
	float exponent;

public:

	LightSpot(glm::vec4 ambi, glm::vec4 diff, glm::vec4 spec,glm::vec3 dir, glm::vec3 pos, float con, float lin, float quad, float cut, float expo);
	void set_cone(float cut, float expo);

};


#endif // !LIGHTS