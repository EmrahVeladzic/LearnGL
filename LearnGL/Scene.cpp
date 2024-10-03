#include "Scene.hpp"

void Scene::AddActor(Actor * newActor) {

	SceneActors.push_back(newActor);

	if (newActor->controlled) {
		Player = newActor;
		
	}
	
	
}

Scene::Scene() {
	Player = nullptr;
}
Scene::~Scene() {
	SceneActors.clear();
}

Scene ActiveScene;
Actor* Player;