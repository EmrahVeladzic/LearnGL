#ifndef RENDERER
#define RENDERER

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Utils.hpp"
#include "Scene.hpp"

extern GLuint shadowBuffer, shadowTex;
extern GLuint renderingProgram;
extern GLuint projLoc, vLoc, tfLoc, mvLoc, invBindLoc, transLoc, offsetLoc, clutMultLoc, tWidthLoc, tHeightLoc, psxShaderLoc;
extern int width, height;
extern float aspect, timeFactor;
extern glm::mat4 pMat, mMat, mvMat;
extern GLuint globalAmbiLoc, ambiLoc, diffLoc, specLoc, posLoc, mAmbiLoc, mDiffLoc, mSpecLoc, mShinLoc;

extern LightGeneric ambient;
extern LightPositional light;


void setupVertices(std::vector<Actor*>& models);
void renderer_init(GLFWwindow* window, std::vector<Actor*>& actors);
void lightingConfig(glm::mat4x4& viewMatrix, Material* mat);
void animate(GLFWwindow* window, double currentTime, std::vector<Actor* >& actors);
void display(GLFWwindow* window, std::vector<Actor* >& actors);
void window_reshape_callback(GLFWwindow* window, int newWidth, int newHeight);

#endif // !RENDERER
