#pragma once
#include "stdafx.h"
#include "PlayerSpawn.hpp"
#include "Light.hpp"
#include "settings.hpp"

class ObjectRegistryBase {
public:
	virtual void instantiateGameObject(std::string name, glm::vec3 pos, glm::vec3 rot, glm::vec3 scale, std::vector<std::string> extraArgs) {};

	virtual void instantiateLight(std::string name, glm::vec3 pos, glm::vec3 rot, glm::vec3 scale, std::vector<std::string> extraArgs) {};
};