#pragma once
#include "PlayerBase.hpp"

class PlayerSpawn {
public:
	static inline PlayerBase* player;
	PlayerSpawn(glm::vec3 position, glm::vec3 rotationEA);
};