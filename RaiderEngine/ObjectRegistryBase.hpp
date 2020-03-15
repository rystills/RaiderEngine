#pragma once
#include "stdafx.h"
#include "GameObject.hpp"
#include "Light.hpp"

class ObjectRegistryBase {
public:
	virtual GameObject* instantiateGameObject(std::string name, glm::vec3 pos, glm::vec3 rot, glm::vec3 scale, std::vector<std::string> extraArgs);

	virtual Light* instantiateLight(std::string name, glm::vec3 pos, glm::vec3 rot, glm::vec3 scale, std::vector<std::string> extraArgs);
};