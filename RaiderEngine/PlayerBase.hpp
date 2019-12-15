#pragma once
#include "stdafx.h"
#include "camera.hpp"
// see this page for the physx CCT demo: https://github.com/NVIDIAGameWorks/PhysX/blob/4050bbfdc2699dfab7edbf0393df8ff96bbe06c5/physx/samples/samplecctsharedcode/SampleCCTCameraController.cpp

class PlayerBase {
public:
	PxControllerManager* manager;
	PxCapsuleController* controller;
	float walkSpeed = 300;
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
	void init() {
		// create the controller manager
		manager = PxCreateControllerManager(*gScene);
		// create the player controller
		PxCapsuleControllerDesc desc;
		desc.height = height;
		desc.radius = radius;
		desc.material = gMaterial;
		controller = (PxCapsuleController*)manager->createController(desc);
		// set the player controller's user data
		controller->setUserData(this);
		// set non-default raycast filter so that the player is ignored when raycasting
		PxShape* shape;
		controller->getActor()->getShapes(&shape, 1);
		shape->setQueryFilterData(noHitFilterData);
	}

	/*
	set the player's position in world space
	@param pos: the position at which to place the player
	@param relative: whether to add the input position to the current position, or to replace the current position with the input position
	@param isFeetPos: whether we are setting the position of the feet or the position of the controller center
	*/
	void setPos(glm::vec3 pos, bool relative = false, bool isFeetPos = true) {
		if (relative) {
			PxExtendedVec3 curPos = controller->getPosition();
			controller->setPosition(PxExtendedVec3(pos.x + curPos.x, pos.y + curPos.y + (isFeetPos ? height / 2 : 0), pos.z + curPos.z));
		}
		else
			controller->setPosition(PxExtendedVec3(pos.x, pos.y + (isFeetPos ? height / 2 : 0), pos.z));
	}

	/*
	sync the position of the camera with the player's current position
	*/
	void syncCameraPos() {
		PxExtendedVec3 playerPos = controller->getPosition();
		mainCam->Position.x = playerPos.x;
		// camera height should be set to the top of the capsule minus the approximate distance from the top of the head to the eyes
		mainCam->Position.y = playerPos.y + (height * (crouching ? crouchScale : 1) / 2 + radius);
		mainCam->Position.z = playerPos.z;
	}

	/*
	return whether or not the player is currently able to jump
	*/
	bool canJump() {
		PxControllerState cctState;
		controller->getState(cctState);
		return ((cctState.collisionFlags & PxControllerCollisionFlag::eCOLLISION_DOWN) != 0) && (velocity.y <= 0);
	}

	void update(float deltaTime) {
		// normalize camera front to get a constant speed regardless of pitch
		glm::vec3 normalFront = glm::normalize(glm::cross(mainCam->WorldUp, mainCam->Right));
		bool grounded = canJump();
		// movement
		float baseMoveSpeed = walkSpeed, forwardSpeed = 0, strafeSpeed = 0;
		if (mainCam->controllable) {
			baseMoveSpeed = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ? runSpeed : walkSpeed);
			forwardSpeed = (int(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) - int(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)) * baseMoveSpeed;
			strafeSpeed = (int(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) - int(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)) * baseMoveSpeed;
			if (forwardSpeed && strafeSpeed) {
				// average forward and strafe speeds to prevent diagonal movement from being faster
				float invMag = baseMoveSpeed / sqrt(forwardSpeed * forwardSpeed + strafeSpeed * strafeSpeed);
				forwardSpeed *= invMag;
				strafeSpeed *= invMag;
			}
		}
		velocity += normalFront * forwardSpeed * (grounded ? 1 : airControl) * deltaTime;
		velocity += mainCam->Right * strafeSpeed * (grounded ? 1 : airControl) * deltaTime;
		// As long as we're grounded, keep vertical velocity at a few ticks of gravity. If we're airborn, continually apply gravity until we return to the ground
		// TODO: keep the player tethered to slopes / steps without relying on an artificial gravity
		if (grounded)
			velocity.y = -playerGravity * 10 * deltaTime;
		else
			velocity.y -= playerGravity * deltaTime;

		// cap horizontal velocity depending on whether the player is walking or running
		float maxMoveSpeed = baseMoveSpeed * maxMoveSpeedRatio;
		float moveVel = sqrt(velocity.x * velocity.x + velocity.z * velocity.z);
		if (moveVel > maxMoveSpeed) {
			float velDiff = (maxMoveSpeed) / moveVel;
			velocity.x *= velDiff;
			velocity.z *= velDiff;
		}
		// if the player is not inputting movement in any direction, slow them down or stop them entirely
		if (forwardSpeed == 0 && strafeSpeed == 0) {
			float stopSpeed = (grounded ? groundStoppingSpeed : airStoppingSpeed) * deltaTime;
			moveVel = sqrt(velocity.x * velocity.x + velocity.z * velocity.z);
			if (moveVel <= stopSpeed) {
				velocity.x = 0;
				velocity.z = 0;
			}
			else {
				float velDir = std::atan2(velocity.z, velocity.x);
				velocity.x -= cos(velDir) * stopSpeed;
				velocity.z -= sin(velDir) * stopSpeed;
			}
		}
		// jump
		if (mainCam->controllable)
			if (grounded && glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
				// jump velocity is a burst, so deltaTime is ignored
				velocity.y = jumpStrength;
		// TODO: set velocity to 0 if the player bumps their head on a ceiling

		// apply to controller
		PxVec3 physVelocity(velocity.x*deltaTime, velocity.y * deltaTime, velocity.z * deltaTime);
		controller->move(physVelocity, 0, deltaTime, NULL);
		// crouch toggle
		if (keyStates[GLFW_KEY_LEFT_CONTROL][pressed]) {
			// we can always crouch, but if we're trying to stand back up, make sure we won't hit our head on something
			if (crouching) {
				PxSceneReadLock scopedLock(*gScene);
				PxCapsuleGeometry geom(radius,height/2);
				PxExtendedVec3 position = controller->getPosition();
				PxVec3 pos((float)position.x, (float)position.y + ((height - height * crouchScale) / 2), (float)position.z);
				PxQuat orientation(PxHalfPi, PxVec3(0.0f, 0.0f, 1.0f));

				PxOverlapBuffer hit;
				if (!gScene->overlap(geom, PxTransform(pos, orientation), hit, PxQueryFilterData(defaultFilterData, PxQueryFlag::eANY_HIT | PxQueryFlag::eSTATIC | PxQueryFlag::eDYNAMIC)))
					goto toggleCrouch;
			}
			else {
			toggleCrouch:
				crouching = !crouching;
				controller->resize(height * (crouching ? crouchScale : 1));
			}
		}
		syncCameraPos();
		mainCam->updateViewProj();
	}
};