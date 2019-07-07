#pragma once
#include "stdafx.h"
#include "camera.hpp"

class Player {
public:
	Camera camera;
	Player() : camera(glm::vec3(0)) { }

	/*
	initialize the player, creating a new newton controller
	*/
	void init() {
		// configure the starting location
		/*dMatrix location(dGetIdentityMatrix());
		location.m_posit.m_y = 0;
		BasicPlayerControllerManager* const playerManager = new BasicPlayerControllerManager(world);
		controller = playerManager->CreatePlayer(location, PLAYER_HEIGHT, PLAYER_RADIUS, PLAYER_MASS);
		playerManager->SetAsPlayer(controller);
		// set the user data
		NewtonBodySetUserData(controller->GetBody(), this);*/
	}

	/*
	set the player's position in world space
	@param pos: the position at which to place the player
	@param relative: whether to add the input position to the current position, or to replace the current position with the input position
	*/
	void setPos(glm::vec3 pos, bool relative = false) {
		/*// TODO: you may need to freeze the world before modifying the player's position; it's probably safe regardless at least when used initially by PlayerSpawn
		NewtonBody* bod = controller->GetBody();
		dMatrix mat;
		NewtonBodyGetMatrix(bod, &mat[0][0]);
		if (relative) {
			mat.m_posit.m_x += pos.x;
			mat.m_posit.m_y += pos.y;
			mat.m_posit.m_z += pos.z;
		}
		else {
			mat.m_posit.m_x = pos.x;
			mat.m_posit.m_y = pos.y;
			mat.m_posit.m_z = pos.z;
		}
		NewtonBodySetMatrix(bod, &mat[0][0]);*/
	}

	/*
	sync the position of the camera with the player's current position
	*/
	/*void syncCameraPos(dFloat* pos) {
		camera.Position.x = pos[0];
		// camera height should be set to the top of the capsule minus the approximate distance from the top of the head to the eyes
		camera.Position.y = pos[1] + (PLAYER_HEIGHT * (crouching ? 1 - crouchScale : 1) - .12f);
		camera.Position.z = pos[2];

	}*/

	void update(float deltaTime) {
		/*NewtonBody* const body = controller->GetBody();
		dFloat pos[4];
		NewtonBodyGetPosition(body, pos);
		*/
		// resync the camera position
		//syncCameraPos(pos);
		camera.moveFlycam();
		camera.updateViewProj();
	}
};