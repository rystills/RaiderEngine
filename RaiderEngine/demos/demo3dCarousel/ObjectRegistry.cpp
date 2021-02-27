#include "stdafx.h"
#if (COMPILE_DEMO == DEMO_3D_CAROUSEL)
#include "Cog.hpp"
#include "FoliageGrass.hpp"
#include "Light.hpp"
#include "FlickerLight.hpp"
#include "settings.hpp"
#include "ObjectRegistry.hpp"
#include "mapLoader.hpp"

GameObject* ObjectRegistry::instantiateGameObject(std::string name, glm::vec3 pos, glm::vec3 rot, glm::vec3 scale, std::vector<std::string> extraArgs) {
	if (name == "Cog") return addGameObject(new Cog(pos, rot, scale, stof(extraArgs[0]), std::stoi(extraArgs[1]), extraArgs.size() > 2 ? std::stoi(extraArgs[2]) : 1));
	if (name == "FoliageGrass") return addGameObject(new FoliageGrass(pos, rot, scale));
	return ObjectRegistryBase::instantiateGameObject(name, pos, rot, scale, extraArgs);
}

Light* ObjectRegistry::instantiateLight(std::string name, glm::vec3 pos, glm::vec3 rot, glm::vec3 scale, std::vector<std::string> extraArgs) {
	if (name == "Flicker") return addLight(new FlickerLight(pos, mapNodeFlags.enableShadows, extraArgs.size() > 0 ? std::stof(extraArgs[0]) : 200, extraArgs.size() > 3 ? glm::vec3(std::stof(extraArgs[1]), std::stof(extraArgs[2]), std::stof(extraArgs[3])) : glm::vec3(1, 1, 1)));
	return ObjectRegistryBase::instantiateLight(name, pos, rot, scale, extraArgs);
}
#endif