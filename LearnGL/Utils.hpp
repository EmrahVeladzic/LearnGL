#pragma once

#include<GL/glew.h>
#include <string>
#include<SOIL2/SOIL2.h>
#include<glm/gtx/matrix_decompose.hpp>
#include"Animation.hpp"



class Utils {

public:

	

	static void printShaderLog(GLuint shader);

	static void printProgramLog(int prog);

	static bool checkOpenGLError();

	static std::string readShaderSource(const char* filePath);

	static GLuint createShaderProgram();

	static void UpdateInterpolationIndex(animJoint& in, float timestamp);
	

	static GLuint createShaderProgram(const char* vp, const char* fp);

	static float getInterpolationValue(float current, float begin, float end);


	static transform matToTrans(glm::mat4x4 input);

	static glm::mat4x4  interpolateTransforms(glm::vec3 transA, glm::vec3 transB, glm::quat rotA, glm::quat rotB, glm::vec3 scalA, glm::vec3 scalB, float passedTrans, float passedRot, float passedScal);

	static glm::mat4x4 transToMat(transform input);


	//static GLuint createShaderProgram(const char* vp, const char* gp, const char* fp);
	//static GLuint createShaderProgram(const char* vp, const char* tCS, const char* tES, const char* fp);
	//static GLuint createShaderProgram(const char* vp, const char* tCS, const char* tES, const char* gp, const char* fp);
	
	
};

