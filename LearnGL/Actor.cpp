#include "Actor.hpp"
#include "Transform.h"


Actor::Actor(const char* fileName, bool p, Camera* cam, transform Trans, float m) {


	Model = ImportedModel(fileName);
	Cam = cam;
	controlled = p;
	T = Trans;
	mvSpeed = m / TARGET_FPS;

}

 void Actor::Move(float Angle) {


	 if (controlled) {		


		 
		 T.rotation.w = Cam->yaw + glm::pi<float>() + Angle;

		 glm::vec4 forwardV = glm::vec4(0.0f,0.0f,mvSpeed,1.0f);

		 glm::mat4x4 rMat = glm::rotate(glm::mat4x4(1.0f), T.rotation.w, glm::vec3(0.0f, 1.0f, 0.0f));

		 forwardV = rMat * forwardV;

		 T.translation += glm::vec3(forwardV);

		 Cam->viewMat = glm::translate(Cam->viewMat, -glm::vec3(forwardV));
		 
		 Cam->Update();
		
	 }

	 else
	 {
		 T.rotation.w = Angle;
	 }

	
 }
