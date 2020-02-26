#include "stdafx.h"
#if (COMPILE_DEMO == DEMO_3D_CAROUSEL)
#include "GameObject.hpp"
#include "Light.hpp"
#include "PlayerSpawn.hpp"
#include "Cog.hpp"
#include "FoliageGrass.hpp"
#include "Light.hpp"
#include "FlickerLight.hpp"
#include "settings.hpp"
#include "ObjectRegistry.hpp"

GameObject* ObjectRegistry::instantiateGameObject(std::string name, glm::vec3 pos, glm::vec3 rot, glm::vec3 scale, std::vector<std::string> extraArgs) {
	if (name == "PlayerSpawn") PlayerSpawn(pos, rot);  // special case: player spawn simply defines the starting transform for the player; don't add it to gameObjects
	else if (name == "Cog") return addGameObject(new Cog(pos, rot, scale, stof(extraArgs[0]), std::stoi(extraArgs[1]), extraArgs.size() > 2 ? std::stoi(extraArgs[2]) : 1));
	else if (name == "FoliageGrass") return addGameObject(new FoliageGrass(pos, rot, scale));
	return NULL;
}

Light* ObjectRegistry::instantiateLight(std::string name, glm::vec3 pos, glm::vec3 rot, glm::vec3 scale, std::vector<std::string> extraArgs) {
	if (name == "point") return addLight(new Light(pos, extraArgs.size() > 0 ? std::stof(extraArgs[0]) : 200, extraArgs.size() > 3 ? glm::vec3(std::stof(extraArgs[1]), std::stof(extraArgs[2]), std::stof(extraArgs[3])) : glm::vec3(1, 1, 1)));
	else if (name == "Flicker") return addLight(new FlickerLight(pos, extraArgs.size() > 0 ? std::stof(extraArgs[0]) : 200, extraArgs.size() > 3 ? glm::vec3(std::stof(extraArgs[1]), std::stof(extraArgs[2]), std::stof(extraArgs[3])) : glm::vec3(1, 1, 1)));
	return NULL;
}
#endif