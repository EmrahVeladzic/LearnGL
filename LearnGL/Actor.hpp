#ifndef ACTOR
#define ACTOR
#include "ModelLoader.hpp"
#include "Transform.h"
#include "Camera.h"

struct Objective
{
	empty target;

	float targetDistance;		

	uint8_t RouteID;

	uint8_t RouteNodeID;

	Objective(empty t = empty(), glm::quat R = glm::quat(1.0f, glm::vec3(0.0f)), float tDist = 1.5f, uint8_t route = 0, uint8_t routeNode = 0);
	
};

struct Actor {

	const char* Asset;

	Camera* Cam;
	transform Trans;
	ImportedModel * Model;

	uint8_t Team;
	
	float mvSpeed;
	
	Objective AI_Node;

	Actor(const char* fileName, Camera* cam = nullptr, transform TRS = transform(), float mv=4.0f, uint8_t team = 0, Objective defaultObjective = Objective());
	
	~Actor();

};



#endif // !ACTOR

