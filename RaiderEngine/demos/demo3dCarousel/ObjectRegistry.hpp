#pragma once
#include "ObjectRegistryBase.hpp"
#include "GameObject.hpp"
#include "Light.hpp"

class ObjectRegistry : public ObjectRegistryBase {
public:
	virtual GameObject* instantiateGameObject(std::string name, glm::vec3 pos, glm::vec3 rot, glm::vec3 scale, std::vector<std::string> extraArgs) override;

	virtual Light* instantiateLight(std::string name, glm::vec3 pos, glm::vec3 rot, glm::vec3 scale, std::vector<std::string> extraArgs) override;
};