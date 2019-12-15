#pragma once
#include "stdafx.h"
#include "PlayerBase.hpp"

class PlayerSpawn {
public:
	static inline PlayerBase* player;
	PlayerSpawn(glm::vec3 position, glm::vec3 rotationEA) {
		// set camera position explicitly first in case we're using a flycam
		mainCam->Position = position;
		if (player == nullptr)
			WARNING(std::cout << "PlayerSpawn instantiated without assigning a valid player pointer; skipping player placement" << std::endl)
		else
			player->setPos(position);
		mainCam->setYaw(glm::degrees(rotationEA.z));
		mainCam->updateCameraVectors();
	}
};