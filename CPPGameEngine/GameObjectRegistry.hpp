#ifndef GAME_OBJECT_REGISTRY_H
#define GAME_OBJECT_REGISTRY_H
#include "GameObject.hpp"
#include "MovingPlatform.hpp"

//TODO: generic arg packing / unpacking
void instantiateGameObject(std::string const& classname, glm::vec3 position, glm::vec3 rotationEA, glm::vec3 scale) {
	if (classname == "MovingPlatform") gameObjects.emplace_back(new MovingPlatform(position, rotationEA, scale));
}

#endif