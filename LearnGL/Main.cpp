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
#define numVBOs 4



float cameraX, cameraY, cameraZ;

GLuint shadowBuffer, shadowTex;

Audio_Handler global_audio;

bool keys[256];


GLuint renderingProgram;
GLuint vao[numVAOs];
GLuint vbo[numVBOs];

GLuint projLoc, vLoc, tfLoc, mvLoc, invBindLoc, transLoc, offsetLoc;
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

LightPositional light(glm::vec4(0.25f, 0.25f, 0.25f, 1.0f),
	glm::vec4(0.7f, 0.7f, 0.7f, 1.0f),
	glm::vec4(0.5f, 0.5f, 0.5f, 1.0f), 
	glm::vec3(5.0f, 5.0f,-5.0f), 
	0.5f, 0.1f, 0.1f);

ALuint sfx;


void setupVertices(std::vector<ImportedModel>& models) {
	

	
	for (ImportedModel& model:models)
	{
		for (size_t i = 0; i < model.Meshes.size(); i++)
		{
			
			

			std::vector<glm::vec3> vert = model.Meshes[i].getVertices();
			std::vector<glm::vec2> tex = model.Meshes[i].getTexCoords();
			std::vector<glm::vec3> norm = model.Meshes[i].getNormals();


			
		

			int numVertices = model.Meshes[i].getNumVertices();
	
		

			for (int j = 0; j < numVertices; j++)
			{
				model.Meshes[i].pvalues.push_back(vert[j].x);
				model.Meshes[i].pvalues.push_back(vert[j].y);
				model.Meshes[i].pvalues.push_back(vert[j].z);



				model.Meshes[i].tvalues.push_back(tex[j].s);
				model.Meshes[i].tvalues.push_back(tex[j].t);

				model.Meshes[i].nvalues.push_back(norm[j].x);
				model.Meshes[i].nvalues.push_back(norm[j].y);
				model.Meshes[i].nvalues.push_back(norm[j].z);

			}


			glGenVertexArrays(1, model.Meshes[i].vao);
			glBindVertexArray(model.Meshes[i].vao[0]);

			glGenBuffers(numVBOs, model.Meshes[i].vbo);

		

			glBindBuffer(GL_ARRAY_BUFFER, model.Meshes[i].vbo[0]);
			glBufferData(GL_ARRAY_BUFFER, model.Meshes[i].pvalues.size() * 4, &model.Meshes[i].pvalues[0], GL_STATIC_DRAW);

			glBindBuffer(GL_ARRAY_BUFFER, model.Meshes[i].vbo[1]);
			glBufferData(GL_ARRAY_BUFFER, model.Meshes[i].tvalues.size() * 4, &model.Meshes[i].tvalues[0], GL_STATIC_DRAW);

			glBindBuffer(GL_ARRAY_BUFFER, model.Meshes[i].vbo[2]);
			glBufferData(GL_ARRAY_BUFFER, model.Meshes[i].nvalues.size() * 4, &model.Meshes[i].nvalues[0], GL_STATIC_DRAW);

		}
	}


}


void init(GLFWwindow* window, std::vector<ImportedModel>& models) {

	renderingProgram = Utils::createShaderProgram("vertShaderCustom.glsl", "fragShaderCustom.glsl");


	glfwGetFramebufferSize(window, &width, &height);
	aspect = (float)width / (float)height;
	pMat = glm::perspective(1.0472f, aspect, 0.1f, 1000.0f);


	cameraX = 0.0f;
	cameraY = 0.5f;
	cameraZ = 8.0f;

	
	setupVertices(models);
		

	ambient.enabled = true;
	light.enabled = true;

	glEnable(GL_MAP_COLOR);
	glEnable(GL_INDEX);




	for (ImportedModel& model : models)
	{		
		
		model.importer.clearData();

	}


	
}

void lightingConfig(glm::mat4x4 viewMatrix) {



	globalAmbiLoc = glGetUniformLocation(renderingProgram, "glo_light.ambient");

	ambiLoc = glGetUniformLocation(renderingProgram, "pos_light.ambient");
	diffLoc = glGetUniformLocation(renderingProgram, "pos_light.diffuse");
	specLoc = glGetUniformLocation(renderingProgram, "pos_light.specular");
	posLoc = glGetUniformLocation(renderingProgram, "pos_light.position");

	mAmbiLoc = glGetUniformLocation(renderingProgram, "material.ambient");
	mDiffLoc = glGetUniformLocation(renderingProgram, "material.diffuse");
	mSpecLoc = glGetUniformLocation(renderingProgram, "material.specular");
	mShinLoc = glGetUniformLocation(renderingProgram, "material.shininess");




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


		

		

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, model.texture);



		for (size_t i = 0; i < model.Meshes.size(); i++)
		{

			
			
			projLoc = glGetUniformLocation(renderingProgram, "proj_matrix");
			glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));

			mvLoc = glGetUniformLocation(renderingProgram, "mv_matrix");
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



void display(GLFWwindow* window ,std::vector<ImportedModel> models) {
	std::stack<glm::mat4> mvStack;

	glClear(GL_DEPTH_BUFFER_BIT);
	glClear(GL_COLOR_BUFFER_BIT);
	
	glClearColor(0.15f,0.15f,0.15f,1.0f);

	glUseProgram(renderingProgram);

	
	

	for (ImportedModel& model : models)
	{
		
		glm::mat4x4 invBTemp = glm::mat4x4(1.0f);
		glm::mat4x4 transTemp = glm::mat4x4(1.0f);

		invBindLoc = glGetUniformLocation(renderingProgram, "inv_bind_matrix");
		transLoc = glGetUniformLocation(renderingProgram, "transform_matrix");


		lightingConfig(vMat);

		

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, model.texture);		

		
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
			glBufferData(GL_ARRAY_BUFFER, model.Meshes[i].pvalues.size() * 4, &model.Meshes[i].pvalues[0], GL_STATIC_DRAW);




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

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model.Meshes[i].vbo[3]);




			glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));



			glUniformMatrix4fv(invBindLoc, 1, GL_FALSE, glm::value_ptr(invBTemp));
			glUniformMatrix4fv(transLoc, 1, GL_FALSE, glm::value_ptr(transTemp));



			glEnable(GL_DEPTH_TEST);
			glEnable(GL_LEQUAL);

			glEnable(GL_ALPHA_TEST);


			glDrawArrays(GL_TRIANGLES, 0, model.Meshes[i].getNumVertices());

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

		Models[0].currentAnim = 2;
	}
	else if (keys[GLFW_KEY_S] == true)
	{
		outtrv.z = 0.1f;

		tmq.w = 0.0f;

		Models[0].currentAnim = 2;
	}

	else if (keys[GLFW_KEY_A] == true)
	{
		outtrv.x = -0.1f;

		tmq.w = -1.5f;

		Models[0].currentAnim = 2;
	}
	else if (keys[GLFW_KEY_D] == true)
	{
		outtrv.x = 0.1f;

		tmq.w = 1.5f;

		Models[0].currentAnim = 2;
	}

	else
	{
		tmq.w = Models[0].rotation.w;

		Models[0].currentAnim = 3;
	}
	
	cameraX += outtrv.x;
	cameraY += outtrv.y;
	cameraZ += outtrv.z;

	Models[0].position += outtrv;
	Models[0].rotation = tmq;
}

void window_reshape_callback(GLFWwindow* window, int newWidth, int newHeight) {
	if (newWidth>0 && newHeight>0)
	{
		aspect = (float)newWidth / (float)newHeight;
		glViewport(0, 0, newWidth, newHeight);
		pMat = glm::perspective(1.0472f, aspect, 0.1f, 1000.0f);
	}
	


}

int main(void) {
	if (!glfwInit()) { exit(EXIT_FAILURE); }

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	

	ALCdevice* audio_device = alcOpenDevice(nullptr);
	ALCcontext* audio_context = alcCreateContext(audio_device,nullptr);
	 
	GLFWwindow* window = glfwCreateWindow(800, 450, "OpenGL3D", NULL, NULL);
	glfwMakeContextCurrent(window);

	alcMakeContextCurrent(audio_context);

	if (!audio_context) {

		exit(EXIT_FAILURE);
	}

	if (glewInit() != GLEW_OK) { exit(EXIT_FAILURE); }
	glfwSwapInterval(1);

	
	Models.push_back(ImportedModel("Dillon.gltf", "Dillon.rpf", glm::vec3(0.0f, 0.0f, 0.0f), glm::quat(0.0f, 0.0f, 1.0f, 0.0f)));
	Models.push_back(ImportedModel("skele.gltf", "skele.rpf", glm::vec3(1.0f, 0.0f, -2.0f),glm::quat(1.8f, 0.0f, 1.0f, 0.0f)));
	Models.push_back(ImportedModel("skele.gltf", "skele.rpf", glm::vec3(-5.0f, 0.0f, -2.0f), glm::quat(0.0f, 0.0f, 1.0f, 0.0f)));
	Models.push_back(ImportedModel("ground.gltf", "ground.rpf", glm::vec3(2.0f, -2.85f, -2.0f), glm::quat((3.14159f/2.0f), 1.0f, 0.0f, 0.0f)));
	

	glfwSetKeyCallback(window,key_callback);

	init(window,Models);

	


	sfx = global_audio.load_WL("Ske.wl");
	

	ALuint src;

	alGenSources(1 ,&src);

	alSourcei(src, AL_BUFFER, sfx);

	alSourcei(src,AL_SOURCE_RELATIVE, AL_TRUE);

	alSourcei(src, AL_LOOPING, AL_TRUE);

	alSourcef(src, AL_GAIN, 0.125f);

	alSourcePlay(src);
	
	
	

	glfwSetWindowSizeCallback(window, window_reshape_callback);


	while (!glfwWindowShouldClose(window))
	{
		
		

		animate(window,glfwGetTime(),Models);
		display(window,Models);
		glfwSwapBuffers(window);
		glfwPollEvents();
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

