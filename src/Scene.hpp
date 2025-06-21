#ifndef SCENE
#define SCENE

#include "Lights.hpp"
#include "AI_Handler.hpp"
#include "Camera.h"
#include "Utils.hpp"

struct Route {

	std::vector<empty> points;
	
};

class Scene {
private:

public:

	std::atomic<bool> performAIUpdates{ true };
	std::mutex AIMutex;

	std::vector<Actor *> SceneActors;
	std::vector<Route> Routes;
	
	void AddActor(Actor * newActor);
	void SwitchPosessedActor(size_t Index = 0);

	void AIUpdate();


	Scene();
	~Scene();

};

extern Actor* Player;
extern Scene ActiveScene;


extern Camera cam;



#endif // !SCENE
