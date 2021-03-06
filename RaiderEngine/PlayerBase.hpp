#pragma once
#include "stdafx.h"
#include "physics.hpp"
#include "GameObject.hpp"

// see this page for the physx CCT demo: https://github.com/NVIDIAGameWorks/PhysX/blob/4050bbfdc2699dfab7edbf0393df8ff96bbe06c5/physx/samples/samplecctsharedcode/SampleCCTCameraController.cpp

class PlayerBase {
public:
	PxControllerManager* manager;
	PxCapsuleController* controller;
	PxRigidDynamic* camBody;
	PxShape* camShape;
	float walkSpeed = 300;
	float crouchSpeed = 180;
	float runSpeed = 480;
	float maxMoveSpeedRatio = 1 / 60.f;
	float height;  // height does not include the half spheres at either end of the capsule; the true height is height + 2 * radius
	float radius;
	float stepHeight = .1f;
	float crouchScale = .1f;
	float eyeTopHeadOffset = .1f;
	float playerGravity = 42;
	float jumpStrength = 9;
	float ladderJumpStrength = 4;
	float ladderJumpSpeed = 10;
	float groundStoppingSpeed = 180;
	float airStoppingSpeed = 12;
	float waterStoppingSpeed = 6;
	float ladderStoppingSpeed = 200;
	float airControl = .1f;
	float waterControl = .3f;
	float ladderControl = 1.f;
	float headBumpDist = .1f;

	glm::vec3 velocity;
	bool crouching = false;
	PxRigidDynamic* waterCheckBody;
	PxShape* waterCheckShape;
	float waterCheckFootOffset = .6f;
	float waterCheckRadius = .1f;
	int swimmingVolumeCount = 0;
	bool swimming = false;
	int underWaterVolumeCount = 0;
	bool underWater = false;
	int ladderVolumeCount = 0;
	GameObject* connectedLadder;
	bool climbingLadder = false;
	bool ctrlDown = false;
	bool flyCam = false;

	PlayerBase() { }

	/*
	initialize the player, creating a new physics controller
	@param inHeight: the height of the player; defaults to 1m
	@param inRadius: the radius of the player; defaults to .4m (for a true height of 1.8m)
	*/
	void init(float inHeight = 1.f, float inRadius = .4f);

	/*
	set the player's position in world space
	@param pos: the position at which to place the player
	@param relative: whether to add the input position to the current position, or to replace the current position with the input position
	@param isFeetPos: whether we are setting the position of the feet or the position of the controller center
	*/
	void setPos(glm::vec3 pos, bool relative = false, bool isFeetPos = true);

	void updateSwimmingVolumeCount(bool enteredNewBody);
	void updateUnderWaterVolumeCount(bool enteredNewBody);
	void updateLadderVolumeCount(bool enteredNewBody, GameObject* ladder);

	/*
	sync the position of the camera with the player's current position
	*/
	void syncCameraPos();
	void syncWaterCheckPos();

	/*
	return whether or not the player is currently able to jump
	*/
	bool canJump();

	void update();
};