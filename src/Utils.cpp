#include "Utils.hpp"
#include <iostream>
#include <string>
#include <fstream>
#include "Animation.h"

extern GLuint vShader;
extern GLuint fShader;

#define INTERPOLATION_APPROX 0.99f

void Utils::UpdateInterpolationIndex(animJoint& in, float timestamp) {
	
	if (in.transIndex >= (int) (in.translations.size() -1) || in.transIndex < 0)
	{
		in.transIndex = 0;
	}
	if (in.rotIndex >= (int) (in.rotations.size() -1) || in.rotIndex < 0 )
	{
		in.rotIndex = 0;
	}
	if (in.scalIndex >= (int) (in.scales.size() -1) || in.scalIndex < 0)
	{
		in.scalIndex = 0;
	}

	

	float beginTrans = in.transTimes[in.transIndex];
	float endTrans = in.transTimes[in.transIndex + 1];

	float beginRot = in.rotTimes[in.rotIndex];
	float endRot = in.rotTimes[in.rotIndex + 1];

	float beginScal = in.scalTimes[in.scalIndex];
	float endScal = in.scalTimes[in.scalIndex + 1];

	in.transInterpolation = getInterpolationValue(timestamp, beginTrans,endTrans);
	in.rotInterpolation = getInterpolationValue(timestamp, beginRot, endRot);
	in.scalInterpolation = getInterpolationValue(timestamp, beginScal, endScal);

	


	if (in.transInterpolation >= INTERPOLATION_APPROX)
	{
		in.transIndex++;
		in.transInterpolation -= INTERPOLATION_APPROX;
	}
	if (in.rotInterpolation >= INTERPOLATION_APPROX)
	{
		in.rotIndex++;
		in.transInterpolation -= INTERPOLATION_APPROX;

	}
	if (in.scalInterpolation >= INTERPOLATION_APPROX)
	{
		in.scalIndex++;
		in.scalInterpolation -= INTERPOLATION_APPROX;
	}



	if (in.transIndex >= (int)(in.translations.size() - 2))
	{
		in.Tend_index = 0;
	}
	else
	{
		in.Tend_index = in.transIndex + 1;
	}


	if (in.rotIndex >= (int)(in.rotations.size() - 2))
	{
		in.Rend_index = 0;
	}
	else
	{
		in.Rend_index = in.rotIndex + 1;
	}

	if (in.scalIndex >= (int)(in.scales.size() - 2))
	{
		in.Send_index = 0;
	}
	else
	{
		in.Send_index = in.scalIndex + 1;
	}

	
}






glm::mat4x4 Utils::interpolateTransforms(glm::vec3 transA, glm::vec3 transB, glm::quat rotA, glm::quat rotB, glm::vec3 scalA, glm::vec3 scalB, float passedTrans, float passedRot, float passedScal) {

	transform out;

	out.translation = glm::mix(transA,transB,passedTrans);
	out.rotation = glm::normalize(glm::slerp(rotA,rotB,passedRot));

	out.scale = glm::mix(scalA, scalB, passedScal);	

	return out.Matrix();
}


float Utils::getInterpolationValue(float current, float begin, float end) {
	
	if (begin > end) {
		end += begin;
	}

	float out = (current - begin) / (end - begin);

	if (out>1.0f)
	{
		out = 1.0f;
	}
	if (out<0.0f)
	{
		out = 0.0f;
	}

	return out;
}



void Utils::printShaderLog(GLuint shader) {
	int len = 0;
	int chWrittn = 0;
	char* log;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
	if (len > 0) {
		log = (char*)malloc(len);
		glGetShaderInfoLog(shader, len, &chWrittn, log);
		std::cout << "Shader Info Log: " << log << std::endl;
		free(log);
	}
}
void Utils::printProgramLog(int prog) {
	int len = 0;
	int chWrittn = 0;
	char* log;
	glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &len);
	if (len > 0) {
		log = (char*)malloc(len);
		glGetProgramInfoLog(prog, len, &chWrittn, log);
		std::cout << "Program Info Log: " << log << std::endl;
		free(log);
	}
}
bool Utils::checkOpenGLError() {
	bool foundError = false;
	int glErr = glGetError();
	while (glErr != GL_NO_ERROR) {
		std::cout << "glError: " << glErr << std::endl;
		foundError = true;
		glErr = glGetError();
	}
	return foundError;
}
std::string Utils::readShaderSource(const char* filePath) {
	std::string content;
	std::ifstream fileStream(filePath);

	std::string line = "";
	while (!fileStream.eof())
	{
		std::getline(fileStream, line);
		content.append(line + "\n");
	}
	fileStream.close();
	return content;
}
GLuint Utils::createShaderProgram() {


	std::string vertShaderStr = readShaderSource("vertShader.glsl");

	std::string fragShaderStr = readShaderSource("fragShader.glsl");

	GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);

	const char* vertShaderSrc = vertShaderStr.c_str() + '\0';
	const char* fragShaderSrc = fragShaderStr.c_str() + '\0';

	glShaderSource(vShader, 1, &vertShaderSrc, NULL);
	glShaderSource(fShader, 1, &fragShaderSrc, NULL);
	glCompileShader(vShader);

	glCompileShader(fShader);

	GLuint vfProgram = glCreateProgram();
	glAttachShader(vfProgram, vShader);
	glAttachShader(vfProgram, fShader);
	glLinkProgram(vfProgram);
	return vfProgram;


}

GLuint Utils::createShaderProgram(const char* vp, const char* fp) {


	std::string vertShaderStr = readShaderSource(vp);

	std::string fragShaderStr = readShaderSource(fp);

	GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);

	const char* vertShaderSrc = vertShaderStr.c_str() + '\0';
	const char* fragShaderSrc = fragShaderStr.c_str() + '\0';

	glShaderSource(vShader, 1, &vertShaderSrc, NULL);
	glShaderSource(fShader, 1, &fragShaderSrc, NULL);
	glCompileShader(vShader);

	glCompileShader(fShader);



	GLuint vfProgram = glCreateProgram();
	glAttachShader(vfProgram, vShader);
	glAttachShader(vfProgram, fShader);
	glLinkProgram(vfProgram);
	return vfProgram;


}

