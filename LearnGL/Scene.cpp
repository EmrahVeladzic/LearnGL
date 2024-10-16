#include "Scene.hpp"

void Scene::AddActor(Actor * newActor) {

	SceneActors.push_back(newActor);

	if (newActor->Cam!=nullptr) {
		Player = newActor;
		
	}
	
	
}

void Scene::SwitchPosessedActor(size_t Index) {
	
	if (Player != nullptr) {
		Player->Cam = nullptr;
	}

	
	if (SceneActors.size() > 0) {			

		Player = SceneActors[Index];
	
		Player->Cam = &cam;
		cam.Retarget(&Player->Trans);

	}
}

Scene::Scene() {
	Player = nullptr;
}
Scene::~Scene() {

	for(Actor* & actor : SceneActors)
	{
		actor->~Actor();
	}

	SceneActors.clear();
}

void::Scene::AIUpdate() {

	while (performAIUpdates)
	{	
		std::chrono::time_point<std::chrono::high_resolution_clock> frameStart = std::chrono::high_resolution_clock::now();
		

		for (Actor *& actor: SceneActors)
		{
			if (actor!=Player)
			{
				AI::Move(actor);
			}

		}

		std::chrono::time_point<std::chrono::high_resolution_clock> frameEnd = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> frameDuration = frameEnd - frameStart;

		double elapsed = frameDuration.count();
		if (elapsed < (1.0f / (float)TARGET_FPS)) {
			std::this_thread::sleep_for(std::chrono::duration<double>(1.0f / ((float)TARGET_FPS) - elapsed));
		}

	}
}


Scene ActiveScene;
Actor* Player;
Camera cam;
