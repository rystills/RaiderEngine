#ifndef MOVING_PLATFORM_H
#define MOVING_PLATFORM_H

#include "GameObject.h"

class MovingPlatform : public GameObject
{
public:
	MovingPlatform(glm::vec3 position, glm::vec3 rotationEA, glm::vec3 scale, std::string modelName, int gameObjectIndex) :
		GameObject(position, rotationEA, scale, modelName, gameObjectIndex) {
	}
	
	void update() override {
		GameObject::update();
	}
};

#endif