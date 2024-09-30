#ifndef CHARACTER
#define CHARACTER
#include "ModelLoader.hpp"
#include "Transform.h"
#include "Camera.h"

class Character {

private:
public:

	Camera* Cam;
	transform T;
	ImportedModel Model;
	bool controlled;

	Character(const char* filePath, transform Trans = transform(), bool p = false, Camera* cam = nullptr);
	
	void Move(glm::vec2 Analog);

};



#endif // !CHARACTER

