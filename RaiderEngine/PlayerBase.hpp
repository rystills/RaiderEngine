#pragma once
#include "stdafx.h"
#include "physics.hpp"

// see this page for the physx CCT demo: https://github.com/NVIDIAGameWorks/PhysX/blob/4050bbfdc2699dfab7edbf0393df8ff96bbe06c5/physx/samples/samplecctsharedcode/SampleCCTCameraController.cpp

class PlayerBase {
public:
	PxControllerManager* manager;
	PxCapsuleController* controller;
	float walkSpeed = 300;
	float crouchSpeed = 180;
	float runSpeed = 480;
	float maxMoveSpeedRatio = 1 / 60.f;
	float height = 1;
	float crouchScale = .3f;
	float radius = .5f;
	float playerGravity = 42;
	float jumpStrength = 9;
	float groundStoppingSpeed = 180;
	float airStoppingSpeed = 12;
	float airControl = .1f;

	glm::vec3 velocity;
	bool crouching = false;
	bool ctrlDown = false;

	PlayerBase() { }

	/*
	initialize the player, creating a new physics controller
	*/
	void init();

	/*
	set the player's position in world space
	@param pos: the position at which to place the player
	@param relative: whether to add the input position to the current position, or to replace the current position with the input position
	@param isFeetPos: whether we are setting the position of the feet or the position of the controller center
	*/
	void setPos(glm::vec3 pos, bool relative = false, bool isFeetPos = true);

	/*
	sync the position of the camera with the player's current position
	*/
	void syncCameraPos();

	/*
	return whether or not the player is currently able to jump
	*/
	bool canJump();

	void update();
};