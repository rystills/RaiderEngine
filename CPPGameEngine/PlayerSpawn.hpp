#pragma once
#include "stdafx.h"
#include "Player.hpp"
extern Player player;

class PlayerSpawn {
public:
	PlayerSpawn(glm::vec3 position, glm::vec3 rotationEA) {
		player.setPos(position);
		player.camera.Yaw = glm::degrees(rotationEA.z);
		player.camera.updateCameraVectors();
	}
};