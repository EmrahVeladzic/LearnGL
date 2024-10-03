#ifndef SCENE
#define SCENE

#include "Lights.hpp"
#include "Actor.hpp"
#include "Camera.h"

class Scene {
private:

public:

	std::vector<Actor *> SceneActors;

	
	void AddActor(Actor * newActor);

	Scene();
	~Scene();

};

extern Actor* Player;
extern Scene ActiveScene;




#endif // !SCENE
