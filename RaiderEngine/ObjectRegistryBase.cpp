#include "stdafx.h"
#include "ObjectRegistryBase.hpp"
#include "GameObject.hpp"
#include "Light.hpp"
#include "PlayerSpawn.hpp"
#include "settings.hpp"
#include "mapLoader.hpp"

GameObject* ObjectRegistryBase::instantiateGameObject(std::string name, glm::vec3 pos, glm::vec3 rot, glm::vec3 scale, std::vector<std::string> extraArgs) {
	if (name == "PlayerSpawn") PlayerSpawn(pos, rot);  // special case: player spawn simply defines the starting transform for the player; don't add it to gameObjects
	else return addGameObject(new GameObject(pos, rot, scale, name, 0, true, mapNodeFlags.usePhysics, mapNodeFlags.castShadows));  // default; spawn a named model as a generic GameObject
	return NULL;
}

Light* ObjectRegistryBase::instantiateLight(std::string name, glm::vec3 pos, glm::vec3 rot, glm::vec3 scale, std::vector<std::string> extraArgs) {
	if (name == "point") return addLight(new Light(pos, extraArgs.size() > 0 ? std::stof(extraArgs[0]) : 200, extraArgs.size() > 3 ? glm::vec3(std::stof(extraArgs[1]), std::stof(extraArgs[2]), std::stof(extraArgs[3])) : glm::vec3(1, 1, 1)));
	return NULL;
}