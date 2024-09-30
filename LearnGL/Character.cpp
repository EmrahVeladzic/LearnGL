#include "Character.hpp"
#include "Transform.h"

Character::Character(const char* fileName,transform Trans, bool p, Camera* cam) {


	Model = ImportedModel(fileName, Trans.translation,Trans.rotation,Trans.scale);
	Cam = cam;
	controlled = p;

}

 void Character::Move(glm::vec2 Analog) {

}
