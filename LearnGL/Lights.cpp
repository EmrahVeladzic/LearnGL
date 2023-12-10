#include <glm/glm.hpp>
#include "Lights.hpp"


LightGeneric::LightGeneric(glm::vec4 ambi, glm::vec4 diff , glm:: vec4 spec) {

	ambient = ambi;
	diffuse = diff;
	specular = spec;
	enabled = false;

	for (int i = 0; i < 4; i++)
	{
		vec4_export[i] = 0.0f;
	}
	for (int i = 0; i < 3; i++)
	{
		vec3_export[i] = 0.0f;
	}
}

glm::vec4 LightGeneric::get_ambient() {
	return ambient;
}
glm::vec4 LightGeneric::get_diffuse() {
	return diffuse;
}
glm::vec4 LightGeneric::get_specular() {
	return specular;
}


float * LightGeneric::get_ambient_float() {

	vec4_export[0] = ambient[0];
	vec4_export[1] = ambient[1];
	vec4_export[2] = ambient[2];
	vec4_export[3] = ambient[3];


	return vec4_export;
}
float * LightGeneric::get_diffuse_float() {

	vec4_export[0] = diffuse[0];
	vec4_export[1] = diffuse[1];
	vec4_export[2] = diffuse[2];
	vec4_export[3] = diffuse[3];

	return vec4_export;
}

float * LightGeneric::get_specular_float() {

	vec4_export[0] = specular[0];
	vec4_export[1] = specular[1];
	vec4_export[2] = specular[2];
	vec4_export[3] = specular[3];

	return vec4_export;
}



glm::vec3 LightDirectional::get_direction() {
	return direction;
}

float* LightDirectional::get_direction_float() {
	vec3_export[0] = direction[0];
	vec3_export[1] = direction[1];
	vec3_export[2] = direction[2];

	return vec3_export;
}



LightDirectional::LightDirectional(glm::vec4 ambi, glm::vec4 diff, glm::vec4 spec, glm::vec3 dir) : LightGeneric(ambi,diff,spec) {

	direction = dir;
}

void LightDirectional::set_direction(glm::vec3 dir) {
	direction = dir;
}

glm::vec3 LightPositional::get_position() {
	return position;
}

float* LightPositional::get_position_float(){
	vec3_export[0] = position[0];
	vec3_export[1] = position[1];
	vec3_export[2] = position[2];

	return vec3_export;
}

float LightPositional::calculate_attenuation(float con, float lin, float quad) {

	return 1.0f / (con+lin+quad);
}

void LightPositional::set_position(glm::vec3 pos) {
	position = pos;
}


	
LightPositional::LightPositional(glm::vec4 ambi, glm::vec4 diff, glm::vec4 spec, glm::vec3 pos, float con, float lin, float quad) :LightGeneric(ambi,diff,spec) {

	position = pos;
	constant = con;
	linear = lin;
	quadratic = quad;
	attenuation = calculate_attenuation(con, lin, quad);

}

void LightSpot::set_cone(float cut, float expo) {
	cutoff = cut;
	exponent = expo;
}

LightSpot::LightSpot(glm::vec4 ambi, glm::vec4 diff, glm::vec4 spec, glm::vec3 dir, glm::vec3 pos, float con, float lin, float quad, float cut, float expo) :LightDirectional(ambi,diff,spec,dir), LightPositional(ambi, diff, spec, pos, con, lin, quad) , LightGeneric(ambi,diff,spec){

	cutoff = cut;
	exponent = expo;

}

