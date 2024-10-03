#ifndef ACTOR
#define ACTOR
#include "ModelLoader.hpp"
#include "Transform.h"
#include "Camera.h"



class Actor {

private:
public:

	Camera* Cam;
	transform T;
	ImportedModel Model;
	bool controlled;
	float mvSpeed;

	Actor(const char* filePath, bool p = false, Camera* cam = nullptr, transform Trans = transform(), float mv=2.0f);
	
	void Move(float angle);

};



#endif // !ACTOR

