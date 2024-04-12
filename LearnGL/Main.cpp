#include <GL/glew.h>
#include <SOIL2/soil2.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <fstream>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Utils.hpp"
#include <stack>
#include "ModelLoader.hpp"
#include "Lights.hpp"
#include "Materials.hpp"
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#include "SoundLoader.hpp"
#include <glm/gtx/string_cast.hpp>

#define numVAOs 1
#define numVBOs 3
#define numEBOs 1

#define TARGET_FPS 60
#define SECOND_F 1000.0f



float cameraX, cameraY, cameraZ;

GLuint shadowBuffer, shadowTex;

Audio_Handler global_audio;

bool keys[256];


GLuint renderingProgram;
GLuint vao[numVAOs];
GLuint vbo[numVBOs];

GLuint projLoc, vLoc, tfLoc, mvLoc, invBindLoc, transLoc, offsetLoc, clutMultLoc ,tWidthLoc,tHeightLoc;
int width, height;
float aspect, timeFactor;
glm::mat4 pMat, vMat, mMat, mvMat;


GLuint globalAmbiLoc, ambiLoc, diffLoc, specLoc, posLoc, mAmbiLoc, mDiffLoc, mSpecLoc, mShinLoc ;

std::vector<ImportedModel> Models;


Material testMat(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
	glm::vec4(0.5f, 0.5f, 0.5f, 1.0f),
	glm::vec4(0.25f, 0.25f, 0.25f, 1.0f), 
	0.1f, 0.0f, 0.0f);

LightGeneric ambient(glm::vec4(0.1f, 0.1f, 0.1f, 1.0f),
	glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
	glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

LightPositional light(glm::vec4(0.125f, 0.125f, 0.125f, 1.0f),
	glm::vec4(0.5f, 0.5f, 0.5f, 1.0f),
	glm::vec4(0.25f, 0.25f, 0.25f, 1.0f), 
	glm::vec3(5.0f, 5.0f,-5.0f), 
	0.5f, 0.1f, 0.1f);

ALuint sfx;


void setupVertices(std::vector<ImportedModel>& models) {
	

	
	for (ImportedModel& model:models)
	{
		for (size_t i = 0; i < model.Meshes.size(); i++)
		{
			
			
			glGenVertexArrays(1, model.Meshes[i].vao);
			glBindVertexArray(model.Meshes[i].vao[0]);

			glGenBuffers(numVBOs, model.Meshes[i].vbo);

		

			glBindBuffer(GL_ARRAY_BUFFER, model.Meshes[i].vbo[0]);
			glBufferData(GL_ARRAY_BUFFER, model.Meshes[i].vertices.size() * 12, &model.Meshes[i].vertices.data()[0], GL_STATIC_DRAW);

			glBindBuffer(GL_ARRAY_BUFFER, model.Meshes[i].vbo[1]);
			glBufferData(GL_ARRAY_BUFFER, model.Meshes[i].texCoords.size() * 8, &model.Meshes[i].texCoords.data()[0], GL_STATIC_DRAW);

			glBindBuffer(GL_ARRAY_BUFFER, model.Meshes[i].vbo[2]);
			glBufferData(GL_ARRAY_BUFFER, model.Meshes[i].normalVecs.size() * 12, &model.Meshes[i].normalVecs.data()[0], GL_STATIC_DRAW);


			glGenBuffers(numEBOs, model.Meshes[i].ebo);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model.Meshes[i].ebo[0]);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, model.Meshes[i].indices.size() * 2, &model.Meshes[i].indices[0], GL_STATIC_DRAW);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		}
	}


}


void init(GLFWwindow* window, std::vector<ImportedModel>& models) {

	renderingProgram = Utils::createShaderProgram("vertShader.glsl", "fragShader.glsl");


	glfwGetFramebufferSize(window, &width, &height);
	aspect = (float)width / (float)height;
	pMat = glm::perspective(1.0472f, aspect, 0.1f, 1000.0f);


	cameraX = 0.0f;
	cameraY = 0.5f;
	cameraZ = 8.0f;

	
	setupVertices(models);
		

	ambient.enabled = true;
	light.enabled = true;

	

	glEnable(GL_DEPTH_TEST);
	
	projLoc = glGetUniformLocation(renderingProgram, "proj_matrix");
	globalAmbiLoc = glGetUniformLocation(renderingProgram, "glo_light.ambient");

	ambiLoc = glGetUniformLocation(renderingProgram, "pos_light.ambient");
	diffLoc = glGetUniformLocation(renderingProgram, "pos_light.diffuse");
	specLoc = glGetUniformLocation(renderingProgram, "pos_light.specular");
	posLoc = glGetUniformLocation(renderingProgram, "pos_light.position");

	mAmbiLoc = glGetUniformLocation(renderingProgram, "material.ambient");
	mDiffLoc = glGetUniformLocation(renderingProgram, "material.diffuse");
	mSpecLoc = glGetUniformLocation(renderingProgram, "material.specular");
	mShinLoc = glGetUniformLocation(renderingProgram, "material.shininess");

	invBindLoc = glGetUniformLocation(renderingProgram, "inv_bind_matrix");
	transLoc = glGetUniformLocation(renderingProgram, "transform_matrix");
	mvLoc = glGetUniformLocation(renderingProgram, "mv_matrix");
	clutMultLoc = glGetUniformLocation(renderingProgram, "clut_multiplier");

	tWidthLoc = glGetUniformLocation(renderingProgram, "tex_width");
	tHeightLoc = glGetUniformLocation(renderingProgram, "tex_height");

	


	for (ImportedModel& model : models)
	{		
		
		model.importer.clearData();

	}


	
}

void lightingConfig(glm::mat4x4 & viewMatrix ) {



	


	glProgramUniform4fv(renderingProgram, globalAmbiLoc, 1, ambient.get_ambient_float());

	glProgramUniform3fv(renderingProgram, posLoc, 1, light.get_position_float());
	glProgramUniform4fv(renderingProgram, ambiLoc, 1, light.get_ambient_float());
	glProgramUniform4fv(renderingProgram, diffLoc, 1, light.get_diffuse_float());
	glProgramUniform4fv(renderingProgram, specLoc, 1, light.get_specular_float());

		

	glProgramUniform4fv(renderingProgram, mAmbiLoc, 1, testMat.get_ambient_float());
	glProgramUniform4fv(renderingProgram, mDiffLoc, 1, testMat.get_diffuse_float());
	glProgramUniform4fv(renderingProgram, mSpecLoc, 1, testMat.get_specular_float());
	glProgramUniform1f(renderingProgram, mShinLoc, testMat.get_shininess());




}

void animate(GLFWwindow* window, double currentTime, std::vector<ImportedModel>& models) {
	

	float currentGlobalTime = (float)currentTime;

	float currentFracTime = fmod(currentGlobalTime, 1.0f);

	int trans_ind;
	int rot_ind;
	int scal_ind;

	float trans_inter;
	float rot_inter;
	float scal_inter;

	for (ImportedModel& model : models)
	{
		int curr = model.currentAnim;




		for (size_t i = 0; i < model.Meshes.size(); i++)
		{

			
			
			

			
			glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));

			
			vMat = glm::translate(glm::mat4(1.0f), glm::vec3(-cameraX, -cameraY, -cameraZ));



			if (model.num_anims > 0) {

				Utils::UpdateInterpolationIndex(model.bones[model.Meshes[i].jointIndex].animations[curr], currentGlobalTime);


				trans_ind = model.bones[model.Meshes[i].jointIndex].animations[curr].transIndex;
				rot_ind = model.bones[model.Meshes[i].jointIndex].animations[curr].rotIndex;
				scal_ind = model.bones[model.Meshes[i].jointIndex].animations[curr].scalIndex;

				trans_inter = model.bones[model.Meshes[i].jointIndex].animations[curr].transInterpolation;
				rot_inter = model.bones[model.Meshes[i].jointIndex].animations[curr].rotInterpolation;
				scal_inter = model.bones[model.Meshes[i].jointIndex].animations[curr].scalInterpolation;

				model.bones[model.Meshes[i].jointIndex].TransformMat = glm::mat4x4(1.0f);


				
				int te = model.bones[model.Meshes[i].jointIndex].animations[curr].Tend_index;
				
				int re = model.bones[model.Meshes[i].jointIndex].animations[curr].Rend_index;
				
				int se = model.bones[model.Meshes[i].jointIndex].animations[curr].Send_index;


				model.bones[model.Meshes[i].jointIndex].TransformMat = Utils::interpolateTransforms(
					model.bones[model.Meshes[i].jointIndex].animations[curr].translations[trans_ind],
					model.bones[model.Meshes[i].jointIndex].animations[curr].translations[te],
					model.bones[model.Meshes[i].jointIndex].animations[curr].rotations[rot_ind],
					model.bones[model.Meshes[i].jointIndex].animations[curr].rotations[re],
					model.bones[model.Meshes[i].jointIndex].animations[curr].scales[scal_ind],
					model.bones[model.Meshes[i].jointIndex].animations[curr].scales[se],
					trans_inter, rot_inter, scal_inter
				);

				
			}

		}


		
	}



}



void display(GLFWwindow* window ,std::vector<ImportedModel>& models) {
	std::stack<glm::mat4> mvStack;

	glClear(GL_DEPTH_BUFFER_BIT);
	glClear(GL_COLOR_BUFFER_BIT);
	
	glClearColor(0.15f,0.15f,0.15f,1.0f);

	glUseProgram(renderingProgram);

	
	

	for (ImportedModel& model : models)
	{
		
		glm::mat4x4 invBTemp = glm::mat4x4(1.0f);
		glm::mat4x4 transTemp = glm::mat4x4(1.0f);

		
		glUniform1f(clutMultLoc, model.clut_multiplier);

		lightingConfig(vMat);

		glUniform1ui(tWidthLoc,model.tex_width);
		glUniform1ui(tHeightLoc, model.tex_height);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, model.texture);		

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_1D, model.clut);


		if (model.num_joints>0)
		{
			model.compute_pose(model.root);
		}

		for (size_t i = 0; i < model.Meshes.size(); i++)
		{

			if (model.num_anims > 0)
			{
				invBTemp = model.INVmatrices[model.Meshes[i].jointIndex];
				transTemp = model.bones[model.Meshes[i].jointIndex].TransformMat;

			}

			



			glBindBuffer(GL_ARRAY_BUFFER, model.Meshes[i].vbo[0]);
			glBufferData(GL_ARRAY_BUFFER, model.Meshes[i].vertices.size() * 12, &model.Meshes[i].vertices.data()[0], GL_STATIC_DRAW);




			mvStack.push(vMat);



			mvStack.push(mvStack.top());
			mvStack.top() *= glm::translate(glm::mat4(1.0f), glm::vec3(model.position.x, model.position.y, model.position.z));

			mvStack.push(mvStack.top());
			mvStack.top() *= glm::rotate(glm::mat4((1.0f)), model.rotation.w, glm::vec3(model.rotation.x, model.rotation.y, model.rotation.z));




			glBindBuffer(GL_ARRAY_BUFFER, model.Meshes[i].vbo[0]);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(0);


			glBindBuffer(GL_ARRAY_BUFFER, model.Meshes[i].vbo[1]);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(1);

			glBindBuffer(GL_ARRAY_BUFFER, model.Meshes[i].vbo[2]);
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(2);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model.Meshes[i].ebo[0]);




			glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));



			glUniformMatrix4fv(invBindLoc, 1, GL_FALSE, glm::value_ptr(invBTemp));
			glUniformMatrix4fv(transLoc, 1, GL_FALSE, glm::value_ptr(transTemp));



		


			
			glDrawElements(GL_TRIANGLES, model.Meshes[i].getNumIndices(), GL_UNSIGNED_SHORT, 0);

		}

	
		
	
	}

	

	while (!mvStack.empty())
	{
		mvStack.pop();
	}

}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	glm::vec3 outtrv = glm::vec3{ 0.0f,0.0f,0.0f };
	glm::quat tmq = glm::quat{0.0f,0.0f,1.0f,0.0f };




	if (action == GLFW_PRESS && key <256 && key>-1)
	{
		keys[key] = true;

	}
	else if (action == GLFW_RELEASE && key < 256 && key>-1)
	{
		keys[key] = false;

	}

	if (keys[GLFW_KEY_W]==true)
	{
		outtrv.z = -0.1f;

		tmq.w = 3.0f;

		if (Models.empty() == false) {

			Models[0].currentAnim = 1;

		}
	}
	else if (keys[GLFW_KEY_S] == true)
	{
		outtrv.z = 0.1f;

		tmq.w = 0.0f;

		if (Models.empty() == false) {

			Models[0].currentAnim = 1;

		}
	}

	else if (keys[GLFW_KEY_A] == true)
	{
		outtrv.x = -0.1f;

		tmq.w = -1.5f;

		if (Models.empty() == false) {

			Models[0].currentAnim = 1;

		}
	}
	else if (keys[GLFW_KEY_D] == true)
	{
		outtrv.x = 0.1f;

		tmq.w = 1.5f;


		if (Models.empty() == false) {
			Models[0].currentAnim = 1;

		}
	}

	else
	{
		tmq.w = Models[0].rotation.w;


		if (Models.empty() == false) {


			Models[0].currentAnim = 0;

		}
	}
	
	cameraX += outtrv.x;
	cameraY += outtrv.y;
	cameraZ += outtrv.z;

	if (Models.empty() == false) {

		Models[0].position += outtrv;
		Models[0].rotation = tmq;

	}


	
}

void window_reshape_callback(GLFWwindow* window, int newWidth, int newHeight) {
	if (newWidth>0 && newHeight>0)
	{
		aspect = (float)newWidth / (float)newHeight;
		glViewport(0, 0, newWidth, newHeight);
		pMat = glm::perspective(1.0472f, aspect, 0.1f, 1000.0f);
	}
	


}

void GLAPIENTRY
MessageCallback(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam)
{


	/*fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
		(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
		type, severity, message);
		*/

}

int main(void) {
	if (!glfwInit()) { exit(EXIT_FAILURE); }

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

	

	ALCdevice* audio_device = alcOpenDevice(nullptr);
	ALCcontext* audio_context = alcCreateContext(audio_device,nullptr);
	 
	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);

	GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "OpenGL3D", monitor, NULL);
	


	glfwMakeContextCurrent(window);

	alcMakeContextCurrent(audio_context);

	if (!audio_context) {

		exit(EXIT_FAILURE);
	}
	

	if (glewInit() != GLEW_OK) { exit(EXIT_FAILURE); }
	glfwSwapInterval(mode->refreshRate/TARGET_FPS);


	
	
	Models.push_back(ImportedModel("skele.gltf", "skele.rpf", glm::vec3(0.0f, 0.0f, -5.0f), glm::quat(0.0f, 0.0f, 1.0f, 0.0f)));
	Models.push_back(ImportedModel("chara.gltf", "char.rpf", glm::vec3(1.0f, 0.0f, -6.0f),glm::quat(1.8f, 0.0f, 1.0f, 0.0f)));
	Models.push_back(ImportedModel("raven.gltf", "raven.rpf", glm::vec3(-5.0f, 0.0f, -7.0f), glm::quat(0.0f, 0.0f, 1.0f, 0.0f)));
	Models.push_back(ImportedModel("ground.gltf", "char.rpf", glm::vec3(2.0f, -2.85f, 0.0f), glm::quat((3.14159f/2.0f), 1.0f, 0.0f, 0.0f)));
	

	glfwSetKeyCallback(window,key_callback);


	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(MessageCallback, 0);

	init(window,Models);

	


	sfx = global_audio.load_WL("raven.wl");
	

	ALuint src;

	alGenSources(1 ,&src);

	alSourcei(src, AL_BUFFER, sfx);

	alSourcei(src,AL_SOURCE_RELATIVE, AL_TRUE);

	alSourcei(src, AL_LOOPING, AL_TRUE);

	alSourcef(src, AL_GAIN, 0.25f);

	alSourcePlay(src);
	
	
	

	glfwSetWindowSizeCallback(window, window_reshape_callback);


	float beg_time = 0.0f;
	float end_time = 0.0f;

	while (!glfwWindowShouldClose(window)){
		
		beg_time =(float) glfwGetTime();
		end_time += beg_time;

		animate(window,glfwGetTime(),Models);
		display(window,Models);
		glfwSwapBuffers(window);
		glfwPollEvents();

		end_time =(float) glfwGetTime();

		
	}


	

	Models.clear();

	alSourceStop(src);


	alDeleteSources(1, &src);
	alDeleteBuffers(1, &sfx);
	alcDestroyContext(audio_context);
	alcCloseDevice(audio_device);


	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}

