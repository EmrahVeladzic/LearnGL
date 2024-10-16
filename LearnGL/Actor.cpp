#include "Actor.hpp"
#include "Transform.h"


Objective::Objective(empty t , glm::quat R , float tDist, uint8_t route, uint8_t routeNode) {

	target = t;
	targetDistance = tDist;	
	

}

Actor::Actor(const char* fileName, Camera* cam, transform T, float m,uint8_t team, Objective defaultObjective) {

	Asset = fileName;

	Model = new ImportedModel(fileName);
	Cam = cam;
	Trans = T;
	mvSpeed = m / TARGET_FPS;
	Team = team;
	AI_Node = defaultObjective;

}

Actor::~Actor() {

	delete Model;


}
