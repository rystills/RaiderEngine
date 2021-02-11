#include "stdafx.h"
#include "PlayerSpawn.hpp"
#include "PlayerBase.hpp"
#include "settings.hpp"
#include "terminalColors.hpp"

PlayerSpawn::PlayerSpawn(glm::vec3 position, glm::vec3 rotationEA) {
	// set camera position explicitly first in case we're using a flycam
	mainCam->Position = position;
	if (player == nullptr)
		WARNINGCOLOR(std::cout << "PlayerSpawn instantiated without assigning a valid player pointer; skipping player placement" << std::endl);
	else
		player->setPos(position);
	
	glm::vec2 pitchYaw = GameObject::pitchYawFromEA(rotationEA);
	mainCam->setPitch(pitchYaw[0]);
	mainCam->setYaw(pitchYaw[1]);
	mainCam->updateCameraVectors();
}