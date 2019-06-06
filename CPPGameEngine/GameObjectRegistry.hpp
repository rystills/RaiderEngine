#ifndef GAME_OBJECT_REGISTRY_H
#define GAME_OBJECT_REGISTRY_H
#include "MovingPlatform.hpp"
#include "PlayerSpawn.hpp"

template <typename ... TArgs>
void instantiateGameObject(std::string name, glm::vec3 pos, glm::vec3 rot, glm::vec3 scale, std::vector<std::string> extraArgs) {
	if (name == "MovingPlatform") gameObjects.emplace_back(new MovingPlatform(pos,rot,scale));
	else if (name == "PlayerSpawn")
		// special case: player spawn simply defines the starting transform for the player; don't add it to the gameObject list
		PlayerSpawn(pos,rot);
}

#endif