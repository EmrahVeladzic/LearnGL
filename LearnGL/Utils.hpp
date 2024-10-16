#ifndef UTILS
#define UTILS


#include<GL/glew.h>
#include <string>
#include"Animation.h"
#include "GL_Math.h"
#include <stack>
#include <chrono>
#include <thread>
#include <mutex>

namespace Utils {
	
 
  void printShaderLog(GLuint shader);
 
  void printProgramLog(int prog);
 
  bool checkOpenGLError();
 
  std::string readShaderSource(const char* filePath);
 
  GLuint createShaderProgram();
 
  void UpdateInterpolationIndex(animJoint& in, float timestamp);
 
 
  GLuint createShaderProgram(const char* vp, const char* fp);
 
  float getInterpolationValue(float current, float begin, float end);
 
 
 // transform matToTrans(glm::mat4x4 input);
 
  glm::mat4x4  interpolateTransforms(glm::vec3 transA, glm::vec3 transB, glm::quat rotA, glm::quat rotB, glm::vec3 scalA, glm::vec3 scalB, float passedTrans, float passedRot, float passedScal);
 
 // glm::mat4x4 transToMat(transform input);
 

	//static GLuint createShaderProgram(const char* vp, const char* gp, const char* fp);
	//static GLuint createShaderProgram(const char* vp, const char* tCS, const char* tES, const char* fp);
	//static GLuint createShaderProgram(const char* vp, const char* tCS, const char* tES, const char* gp, const char* fp);
	
	
};


#endif // !UTILS