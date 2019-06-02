#ifndef GAME_OBJECT_REGISTRY_H
#define GAME_OBJECT_REGISTRY_H
#include "GameObject.h"
#include "MovingPlatform.h"

//TODO: generic arg packing / unpacking
void instantiateGameObject(std::string const& classname, glm::vec3 position, glm::vec3 rotationEA, glm::vec3 scale, std::string modelName, int gameObjectIndex) {
	if (classname == "MovingPlatform") gameObjects.emplace_back(new MovingPlatform(position, rotationEA, scale, modelName, gameObjectIndex));
}

#endif