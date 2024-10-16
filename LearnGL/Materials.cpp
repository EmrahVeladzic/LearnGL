#include "GL_Math.h"
#include "Materials.hpp"

Material::Material(glm::vec4 ambi, glm::vec4 diff, glm::vec4 spec, float shin,float trans, float emmis) {
	ambient = ambi;
	diffuse = diff;
	specular = spec;
	shininess = shin;
	transparency = trans;
	emmision = emmis;

	for (int i = 0; i < 4; i++) {

		vec4_export[i] = 0.0f;

	}

}

glm::vec4 Material::get_ambient() {
	return ambient;
}
glm::vec4 Material::get_diffuse() {
	return diffuse;
}
glm::vec4 Material::get_specular() {
	return specular;
}

float* Material::get_ambient_float() {

	vec4_export[0] = ambient[0];
	vec4_export[1] = ambient[1];
	vec4_export[2] = ambient[2];
	vec4_export[3] = ambient[3];

	return vec4_export;
}
float* Material::get_diffuse_float() {

	vec4_export[0] = diffuse[0];
	vec4_export[1] = diffuse[1];
	vec4_export[2] = diffuse[2];
	vec4_export[3] = diffuse[3];

	return vec4_export;
}
float* Material::get_specular_float() {

	vec4_export[0] = specular[0];
	vec4_export[1] = specular[1];
	vec4_export[2] = specular[2];
	vec4_export[3] = specular[3];

	return vec4_export;
}


float Material::get_shininess() {
	return shininess;
}
float Material::get_transparency() {
	return transparency;
}
float Material::get_emmision() {
	return emmision;
}