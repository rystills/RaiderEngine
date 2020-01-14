#pragma once
#include "ObjectRegistryBase.hpp"

class ObjectRegistry : public ObjectRegistryBase {
public:
	virtual void instantiateGameObject(std::string name, glm::vec3 pos, glm::vec3 rot, glm::vec3 scale, std::vector<std::string> extraArgs) override;

	virtual void instantiateLight(std::string name, glm::vec3 pos, glm::vec3 rot, glm::vec3 scale, std::vector<std::string> extraArgs) override;
};