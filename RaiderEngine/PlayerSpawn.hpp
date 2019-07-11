#pragma once
#include "stdafx.h"
#include "Player.hpp"

class PlayerSpawn {
public:
	PlayerSpawn(glm::vec3 position, glm::vec3 rotationEA) {
		// set camera position explicitly first in case we're using a flycam
		player.camera.Position = position;
		player.setPos(position);
		player.camera.setYaw(glm::degrees(rotationEA.z));
		player.camera.updateCameraVectors();
	}
};