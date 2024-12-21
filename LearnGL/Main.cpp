#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "Utils.hpp"
#include "Materials.hpp"
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#include "SoundLoader.hpp"
#include "GL_Math.h"
#include "Scene.hpp"


GLuint shadowBuffer, shadowTex;

Audio_Handler global_audio;

#define NUM_KEYS 300

bool keys[NUM_KEYS];


GLuint renderingProgram;


GLuint projLoc, vLoc, tfLoc, mvLoc, invBindLoc, transLoc, offsetLoc, clutMultLoc ,tWidthLoc,tHeightLoc;
int width, height;
float aspect, timeFactor;
glm::mat4 pMat, mMat, mvMat;


GLuint globalAmbiLoc, ambiLoc, diffLoc, specLoc, posLoc, mAmbiLoc, mDiffLoc, mSpecLoc, mShinLoc ;




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



void setupVertices(std::vector<Actor *>& models) {
	

	
	for (Actor * & actor:models)
	{
		for (size_t i = 0; i < actor->Model->Meshes.size(); i++)
		{
			
			
			glGenVertexArrays(1, actor->Model->Meshes[i].vao);
			glBindVertexArray(actor->Model->Meshes[i].vao[0]);

			glGenBuffers(numVBOs, actor->Model->Meshes[i].vbo);

		

			glBindBuffer(GL_ARRAY_BUFFER, actor->Model->Meshes[i].vbo[0]);
			glBufferData(GL_ARRAY_BUFFER, actor->Model->Meshes[i].vertices.size() * 12, &actor->Model->Meshes[i].vertices.data()[0], GL_STATIC_DRAW);

			glBindBuffer(GL_ARRAY_BUFFER, actor->Model->Meshes[i].vbo[1]);
			glBufferData(GL_ARRAY_BUFFER, actor->Model->Meshes[i].texCoords.size() * 8, &actor->Model->Meshes[i].texCoords.data()[0], GL_STATIC_DRAW);

			glBindBuffer(GL_ARRAY_BUFFER, actor->Model->Meshes[i].vbo[2]);
			glBufferData(GL_ARRAY_BUFFER, actor->Model->Meshes[i].normalVecs.size() * 12, &actor->Model->Meshes[i].normalVecs.data()[0], GL_STATIC_DRAW);


			glGenBuffers(numEBOs, actor->Model->Meshes[i].ebo);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, actor->Model->Meshes[i].ebo[0]);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, actor->Model->Meshes[i].indices.size() * 2, &actor->Model->Meshes[i].indices[0], GL_STATIC_DRAW);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		}
	}


}


void init(GLFWwindow* window, std::vector<Actor *>& actors) {

	renderingProgram = Utils::createShaderProgram("vertShader.glsl", "fragShader.glsl");


	glfwGetFramebufferSize(window, &width, &height);
	aspect = (float)width / (float)height;
	pMat = glm::perspective(1.0472f, aspect, 0.1f, 1000.0f);

	if (Player!=nullptr) {

		cam.Retarget(&Player->Trans);
		
	}
	setupVertices(actors);
		

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

	


	for (Actor * & actor : actors)
	{		
		
		actor->Model->importer.clearData();

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

void animate(GLFWwindow* window, double currentTime, std::vector<Actor * >& actors) {
	

	float currentGlobalTime = (float)(currentTime);

	

	int trans_ind;
	int rot_ind;
	int scal_ind;

	float trans_inter;
	float rot_inter;
	float scal_inter;

	for (Actor* & actor : actors)
	{
		int curr = actor->Model->currentAnim;

		



		for (size_t i = 0; i < actor->Model->Meshes.size(); i++)
		{
			
			
			
			


			if (actor->Model->num_anims > 0) {

				

				Utils::UpdateInterpolationIndex(actor->Model->bones[actor->Model->Meshes[i].jointIndex].animations[curr], currentGlobalTime);


				trans_ind = actor->Model->bones[actor->Model->Meshes[i].jointIndex].animations[curr].transIndex;
				rot_ind = actor->Model->bones[actor->Model->Meshes[i].jointIndex].animations[curr].rotIndex;
				scal_ind = actor->Model->bones[actor->Model->Meshes[i].jointIndex].animations[curr].scalIndex;

				trans_inter = actor->Model->bones[actor->Model->Meshes[i].jointIndex].animations[curr].transInterpolation;
				rot_inter = actor->Model->bones[actor->Model->Meshes[i].jointIndex].animations[curr].rotInterpolation;
				scal_inter = actor->Model->bones[actor->Model->Meshes[i].jointIndex].animations[curr].scalInterpolation;

				
				
				int te = actor->Model->bones[actor->Model->Meshes[i].jointIndex].animations[curr].Tend_index;
				
				int re = actor->Model->bones[actor->Model->Meshes[i].jointIndex].animations[curr].Rend_index;
				
				int se = actor->Model->bones[actor->Model->Meshes[i].jointIndex].animations[curr].Send_index;

				



				actor->Model->bones[actor->Model->Meshes[i].jointIndex].TransformMat = Utils::interpolateTransforms(
					actor->Model->bones[actor->Model->Meshes[i].jointIndex].animations[curr].translations[trans_ind],
					actor->Model->bones[actor->Model->Meshes[i].jointIndex].animations[curr].translations[te],
					actor->Model->bones[actor->Model->Meshes[i].jointIndex].animations[curr].rotations[rot_ind],
					actor->Model->bones[actor->Model->Meshes[i].jointIndex].animations[curr].rotations[re],
					actor->Model->bones[actor->Model->Meshes[i].jointIndex].animations[curr].scales[scal_ind],
					actor->Model->bones[actor->Model->Meshes[i].jointIndex].animations[curr].scales[se],
					trans_inter, rot_inter, scal_inter
				);
				
				
			}

		}




		
	}



}



void display(GLFWwindow* window ,std::vector<Actor * >& actors) {
	std::stack<glm::mat4> mvStack;

	glClear(GL_DEPTH_BUFFER_BIT);
	glClear(GL_COLOR_BUFFER_BIT);
	
	glClearColor(0.25f, 0.25f, 0.25f,1.0f);

	glUseProgram(renderingProgram);

	
	

	for (Actor * & actor : actors)
	{
		
		glm::mat4x4 invBTemp = glm::mat4x4(1.0f);
		glm::mat4x4 transTemp = glm::mat4x4(1.0f);

		
		glUniform1f(clutMultLoc, actor->Model->clut_multiplier);

		lightingConfig(cam.viewMat);

		glUniform1ui(tWidthLoc, actor->Model->tex_width);
		glUniform1ui(tHeightLoc, actor->Model->tex_height);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, actor->Model->texture);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_1D, actor->Model->clut);

		

		if (actor->Model->num_joints>0)
		{
			actor->Model->compute_pose(actor->Model->root);
		}

		for (size_t i = 0; i < actor->Model->Meshes.size(); i++)
		{

			if (actor->Model->num_anims > 0)
			{

				if (useAST) {
					invBTemp = glm::mat4x4(1.0f);
				}
				else
				{
					invBTemp = actor->Model->INVmatrices[actor->Model->Meshes[i].jointIndex];
				}
				
				transTemp = actor->Model->bones[actor->Model->Meshes[i].jointIndex].TransformMat;
			}
					


			glBindBuffer(GL_ARRAY_BUFFER, actor->Model->Meshes[i].vbo[0]);
			glBufferData(GL_ARRAY_BUFFER, actor->Model->Meshes[i].vertices.size() * 12, &actor->Model->Meshes[i].vertices.data()[0], GL_STATIC_DRAW);




			mvStack.push(cam.viewMat);

			glm::mat4 translationMat = glm::translate(glm::mat4(1.0f),actor->Trans.translation);

			
			mvStack.push(mvStack.top() * translationMat);


			glm::mat4 rotationMat = glm::mat4_cast(actor->Trans.rotation);

			
			mvStack.push(mvStack.top() * rotationMat);


			glm::mat4 scaleMat = glm::scale(glm::mat4(1.0f), actor->Trans.scale);
			mvStack.push(mvStack.top() * scaleMat);

			glBindBuffer(GL_ARRAY_BUFFER, actor->Model->Meshes[i].vbo[0]);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(0);


			glBindBuffer(GL_ARRAY_BUFFER, actor->Model->Meshes[i].vbo[1]);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(1);

			glBindBuffer(GL_ARRAY_BUFFER, actor->Model->Meshes[i].vbo[2]);
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(2);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, actor->Model->Meshes[i].ebo[0]);




			glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));



			glUniformMatrix4fv(invBindLoc, 1, GL_FALSE, glm::value_ptr(invBTemp));
			glUniformMatrix4fv(transLoc, 1, GL_FALSE, glm::value_ptr(transTemp));




			glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));
			


			
			glDrawElements(GL_TRIANGLES, actor->Model->Meshes[i].getNumIndices(), GL_UNSIGNED_SHORT, 0);

		}

	
		
	
	}

	

	while (!mvStack.empty())
	{
		mvStack.pop();
	}

}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	
	

	if (Player!=nullptr) {

		
			


		if (action == GLFW_PRESS && key <NUM_KEYS && key>-1)
		{
			keys[key] = true;

		}
		else if (action == GLFW_RELEASE && key < NUM_KEYS && key>-1)
		{
			keys[key] = false;

		}

		


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


void KeyActions() {

	if (Player != nullptr) {

		if (keys[GLFW_KEY_W] == true)
		{
			
			AI::Move(Player,glm::two_pi<float>());

		}
		else if (keys[GLFW_KEY_S] == true)
		{
			AI::Move(Player,glm::pi<float>());
			


		}

		else if (keys[GLFW_KEY_A] == true)
		{
					

			AI::Move(Player,glm::half_pi<float>());


		}
		else if (keys[GLFW_KEY_D] == true)
		{
			AI::Move(Player,glm::pi<float>()+glm::half_pi<float>());

		}




		else
		{


			Player->Model->currentAnim = 0;

		}


		if (keys[GLFW_KEY_LEFT]) {

			cam.X_Axis(1.0f);


		}

		if (keys[GLFW_KEY_RIGHT]) {


			cam.X_Axis(-1.0f);


		}

		if (keys[GLFW_KEY_UP]) {

			cam.Y_Axis(-1.0f);
		}

		if (keys[GLFW_KEY_DOWN]) {

			cam.Y_Axis(1.0f);
		}
		

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

	glm::ivec2 resolution = glm::vec2(mode->width, mode->height);

	if (!FULLSCREEN) {
		monitor = NULL;
		resolution.x = 1600;
		resolution.y = 1200;
	}
	
	GLFWwindow* window = glfwCreateWindow(resolution.x, resolution.y, "OpenGL3D", monitor, NULL);
	
	glfwMakeContextCurrent(window);

	alcMakeContextCurrent(audio_context);

	if (!audio_context) {

		exit(EXIT_FAILURE);

	}
	


	if (glewInit() != GLEW_OK) { exit(EXIT_FAILURE); }

	
	glfwSwapInterval(1);

		
	ActiveScene.AddActor(new Actor("chara", &cam));


	ActiveScene.AddActor(new Actor("skele", NULL, transform(glm::vec3(1.0f,0.0f,10.0f)),2.0f));

	ActiveScene.AddActor(new Actor("raven", NULL, transform(glm::vec3(4.0f, 0.0f, 16.0f)), 0.0f));

	glfwSetKeyCallback(window,key_callback);

	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(MessageCallback, 0);

	init(window, ActiveScene.SceneActors);

	sfx = global_audio.load_WL("rifftape");

	ALuint src;

	alGenSources(1 ,&src);

	alSourcei(src, AL_BUFFER, sfx);

	alSourcei(src,AL_SOURCE_RELATIVE, AL_TRUE);

	alSourcei(src, AL_LOOPING, AL_TRUE);

	alSourcef(src, AL_GAIN, 1.0f);

	alSourcePlay(src);
	
	glfwSetWindowSizeCallback(window, window_reshape_callback);


	
	std::thread AIThread(&Scene::AIUpdate,&ActiveScene);
	
	AIThread.detach();

	while (!glfwWindowShouldClose(window)){		
		
		std::chrono::time_point<std::chrono::high_resolution_clock> frameStart = std::chrono::high_resolution_clock::now();


		animate(window,glfwGetTime(), ActiveScene.SceneActors);
		display(window, ActiveScene.SceneActors);
		glfwSwapBuffers(window);
		glfwPollEvents();
		KeyActions();

		
		ActiveScene.SceneActors[1]->AI_Node.target = Player->Trans;
	
	
		std::chrono::time_point<std::chrono::high_resolution_clock> frameEnd = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> frameDuration = frameEnd - frameStart;

		double elapsed = frameDuration.count();
		if (elapsed < (1.0f/(float)TARGET_FPS)) {
			std::this_thread::sleep_for(std::chrono::duration<double>(1.0f / ((float)TARGET_FPS) - elapsed));
		}

	}

	ActiveScene.performAIUpdates = false;
	
	alSourceStop(src);
	
	
	alDeleteSources(1, &src);
	alDeleteBuffers(1, &sfx);
	alcDestroyContext(audio_context);
	alcCloseDevice(audio_device);

	

	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}

