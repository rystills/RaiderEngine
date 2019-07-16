#pragma once
#include "stdafx.h"
#include "camera.hpp"
// see this page for the physx CCT demo: https://github.com/NVIDIAGameWorks/PhysX/blob/4050bbfdc2699dfab7edbf0393df8ff96bbe06c5/physx/samples/samplecctsharedcode/SampleCCTCameraController.cpp

class Player {
public:
	Camera camera;
	PxControllerManager* manager;
	PxController* controller;
	float walkSpeed = 5;
	float runSpeed = 8;
	float height = 1.9f;
	float crouchScale = .3f;
	float radius = .5f;
	float playerGravity = .7f;
	float jumpStrength = .14f;
	float groundStoppingSpeed = 3;
	float airStoppingSpeed = .2f;
	float airControl = .1f;
	
	glm::vec3 velocity;
	bool crouching = false;
	bool ctrlDown = false;

	Player() : camera(glm::vec3(0)) { }

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
		controller = manager->createController(desc);
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
			controller->setPosition(PxExtendedVec3(pos.x + curPos.x, pos.y + curPos.y + (isFeetPos ? height/2 : 0), pos.z + curPos.z));
		}
		else
			controller->setPosition(PxExtendedVec3(pos.x, pos.y + (isFeetPos ? height / 2 : 0), pos.z));
	}

	/*
	sync the position of the camera with the player's current position
	*/
	void syncCameraPos() {
		PxExtendedVec3 playerPos = controller->getPosition();
		camera.Position.x = playerPos.x;
		// camera height should be set to the top of the capsule minus the approximate distance from the top of the head to the eyes
		camera.Position.y = playerPos.y + (height * (crouching ? 1 - crouchScale : 1) / 2 - .12f);
		camera.Position.z = playerPos.z;
	}
	
	/*
	return whether or not the player is currently able to jump
	*/
	bool canJump() {
		PxControllerState cctState;
		controller->getState(cctState);
		return (cctState.collisionFlags & PxControllerCollisionFlag::eCOLLISION_DOWN) != 0;
	}

	void update(float deltaTime) {
		if (camera.controllable) {
			bool grounded = canJump();
			// normalize camera front to get a constant speed regardless of pitch
			glm::vec3 normalFront = glm::normalize(glm::cross(camera.WorldUp, camera.Right));
			// movement
			float baseMoveSpeed = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ? runSpeed : walkSpeed);
			float forwardSpeed = (int(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) - int(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)) * baseMoveSpeed;
			float strafeSpeed = (int(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) - int(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)) * baseMoveSpeed;
			if (forwardSpeed && strafeSpeed) {
				// average forward and strafe speeds to prevent diagonal movement from being faster
				float invMag = baseMoveSpeed / sqrt(forwardSpeed * forwardSpeed + strafeSpeed * strafeSpeed);
				forwardSpeed *= invMag;
				strafeSpeed *= invMag;
			}
			velocity += normalFront * forwardSpeed * (grounded ? 1 : airControl) * deltaTime;
			velocity += camera.Right * strafeSpeed * (grounded ? 1 : airControl) * deltaTime;
			// As long as we're grounded, keep vertical velocity at a few ticks of gravity. If we're airborn, continually apply gravity until we return to the ground
			// TODO: keep the player tethered to slopes / steps without relying on an artificial gravity
			if (grounded)
				velocity.y = -playerGravity * 10 * deltaTime;
			else
				velocity.y -= playerGravity * deltaTime;
			
			// cap horizontal velocity depending on whether the player is walking or running
			float moveVel = sqrt(velocity.x * velocity.x + velocity.z * velocity.z);
			if (moveVel > baseMoveSpeed * deltaTime) {
				float velDiff = (baseMoveSpeed * deltaTime) / moveVel;
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
			PxControllerState s;
			controller->getState(s);
			if (grounded && glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
				// jump velocity is a burst, so deltaTime is ignored
				velocity.y = jumpStrength;
			// apply to controller
			PxVec3 physVelocity(velocity.x, velocity.y, velocity.z);
			controller->move(physVelocity, 0, deltaTime, NULL);
			// crouch toggle
			if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
				ctrlDown = true;
			else if (ctrlDown) {
				ctrlDown = false;
				// TODO: reimplement crouching. See: https://github.com/NVIDIAGameWorks/PhysX/blob/4050bbfdc2699dfab7edbf0393df8ff96bbe06c5/physx/samples/samplecctsharedcode/SampleCCTActor.cpp#L212
				crouching = !crouching;
			}
		}
		syncCameraPos();
		camera.updateViewProj();
	}
};