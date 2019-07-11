#pragma once
#include "stdafx.h"
#include "MovingPlatform.hpp"
#include "PlayerSpawn.hpp"
#include "Cog.hpp"
#include "Light.hpp"
#include "FlickerLight.hpp"
#include "settings.hpp"

void instantiateGameObject(std::string name, glm::vec3 pos, glm::vec3 rot, glm::vec3 scale, std::vector<std::string> extraArgs) {
	if (name == "MovingPlatform") gameObjects.emplace_back(new MovingPlatform(pos, rot, scale));
	else if (name == "PlayerSpawn") PlayerSpawn(pos, rot);  // special case: player spawn simply defines the starting transform for the player; don't add it to gameObjects
	else if (name == "Cog") gameObjects.emplace_back(new Cog(pos, rot, scale, stof(extraArgs[0]), std::stoi(extraArgs[1]), extraArgs.size() > 2 ? std::stoi(extraArgs[2]) : 1));
}

void instantiateLight(std::string name, glm::vec3 pos, glm::vec3 rot, glm::vec3 scale, std::vector<std::string> extraArgs) {
	if (name == "point") lights.emplace_back(new Light(pos, extraArgs.size() > 0 ? std::stof(extraArgs[0]) : 200, extraArgs.size() > 3 ? glm::vec3(std::stof(extraArgs[1]), std::stof(extraArgs[2]), std::stof(extraArgs[3])) : glm::vec3(1, 1, 1)));
	else if (name == "Flicker") lights.emplace_back(new FlickerLight(pos, extraArgs.size() > 0 ? std::stof(extraArgs[0]) : 200, extraArgs.size() > 3 ? glm::vec3(std::stof(extraArgs[1]), std::stof(extraArgs[2]), std::stof(extraArgs[3])) : glm::vec3(1, 1, 1)));
}