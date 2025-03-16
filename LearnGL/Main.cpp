#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "Utils.hpp"
#include "Renderer.hpp"
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#include "SoundLoader.hpp"
#include "GL_Math.h"
#include "Scene.hpp"

Audio_Handler global_audio;

#define NUM_KEYS 300

bool keys[NUM_KEYS];



ALuint * sfx;


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {



	if (Player != nullptr) {





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


void KeyActions() {

	if (Player != nullptr) {

		if (keys[GLFW_KEY_W] == true)
		{

			AI::Move(Player, glm::two_pi<float>());

		}
		else if (keys[GLFW_KEY_S] == true)
		{
			AI::Move(Player, glm::pi<float>());



		}

		else if (keys[GLFW_KEY_A] == true)
		{


			AI::Move(Player, glm::half_pi<float>());


		}
		else if (keys[GLFW_KEY_D] == true)
		{
			AI::Move(Player, glm::pi<float>() + glm::half_pi<float>());

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
	ALCcontext* audio_context = alcCreateContext(audio_device, nullptr);

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


	ActiveScene.AddActor(new Actor("CHARA", &cam));


	ActiveScene.AddActor(new Actor("SKELE", NULL, transform(glm::vec3(1.0f, 0.0f, 10.0f)), 2.0f));

	ActiveScene.AddActor(new Actor("RAVEN", NULL, transform(glm::vec3(4.0f, 0.0f, 16.0f)), 0.0f));


	glfwSetKeyCallback(window, key_callback);

	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(MessageCallback, 0);

	renderer_init(window, ActiveScene.SceneActors);

	sfx = global_audio.load_WL("RAVEN");

	ALuint src;

	alGenSources(1, &src);

	alSourcei(src, AL_BUFFER, sfx[0]);

	alSourcei(src, AL_SOURCE_RELATIVE, AL_TRUE);

	alSourcei(src, AL_LOOPING, (sfx[1]==1)? AL_TRUE: AL_FALSE);

	alSourcef(src, AL_GAIN, 1.0f);

	alSourcePlay(src);

	glfwSetWindowSizeCallback(window, window_reshape_callback);

	

	std::thread AIThread(&Scene::AIUpdate, &ActiveScene);

	AIThread.detach();

	while (!glfwWindowShouldClose(window)) {

		std::chrono::time_point<std::chrono::high_resolution_clock> frameStart = std::chrono::high_resolution_clock::now();


		animate(window, glfwGetTime(), ActiveScene.SceneActors);
		display(window, ActiveScene.SceneActors);
		glfwSwapBuffers(window);
		glfwPollEvents();
		KeyActions();


		ActiveScene.SceneActors[1]->AI_Node.target = Player->Trans;


		std::chrono::time_point<std::chrono::high_resolution_clock> frameEnd = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> frameDuration = frameEnd - frameStart;

		double elapsed = frameDuration.count();
		if (elapsed < (1.0f / (float)TARGET_FPS)) {
			std::this_thread::sleep_for(std::chrono::duration<double>(1.0f / ((float)TARGET_FPS) - elapsed));
		}

	}

	ActiveScene.performAIUpdates = false;

	alSourceStop(src);


	alDeleteSources(1, &src);
	alDeleteBuffers(1, &sfx[0]);
	alcDestroyContext(audio_context);
	alcCloseDevice(audio_device);

	delete[] sfx;

	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}

