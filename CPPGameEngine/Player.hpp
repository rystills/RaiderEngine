#pragma once

#include "camera.hpp"

class Player {
public:
	Camera camera;
	//kineCon* controller;
	//btPairCachingGhostObject* ghostObject;
	//btCapsuleShape* convexShape;
	float radius = .5f;
	float height = 1.6f;
	float walkSpeed = .02f;
	float runSpeed = .04f;
	float jumpSpeed = 5;
	float maxStepHeight = .05f;

	Player() : camera(glm::vec3(0)) { }

	/*
	initialize the player once Bullet initialization has been completed
	*/
	void init() {
	}

	/*
	set the player's position in world space
	@param pos: the position at which to place the player
	*/
	void setPos(glm::vec3 pos) {
		
	}

	/*
	sync the position of the camera with the player's current position
	*/
	void syncCameraPos() {
		/*btVector3 pos = ghostObject->getWorldTransform().getOrigin();
		camera.Position.x = pos.getX();
		// camera height should be set to the top of the capsule minus the approximate distance from the top of the head to the eyes
		camera.Position.y = pos.getY() + (height / 2 - .12f);
		camera.Position.z = pos.getZ();
		*/
	}

	void update(GLFWwindow *window, float deltaTime) {
		// process input
		// TODO: move this window close block somewhere else
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);

		// normalize camera front to get a constant speed regardless of pitch
		glm::vec3 normalFront = glm::normalize(glm::cross(camera.WorldUp, camera.Right));
		// walk
		glm::vec3 dir(0, 0, 0);
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
			dir += normalFront;
			camera.ProcessKeyboard(FORWARD, deltaTime);
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
			dir -= normalFront;
			camera.ProcessKeyboard(BACKWARD, deltaTime);
		}

		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
			dir -= camera.Right;
			camera.ProcessKeyboard(LEFT, deltaTime);
		}
		
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
			dir += camera.Right;
			camera.ProcessKeyboard(RIGHT, deltaTime);
		}
		camera.updateViewProj();
		/*if (dir == glm::vec3(0, 0, 0))
			controller->setWalkDirection(btVector3(0, 0, 0));
		else
			controller->setWalkDirection(btVector3(dir.x, dir.y, dir.z).normalized()*(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ? runSpeed : walkSpeed));

		// jump
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
			if (controller->canJump())
				controller->jump();
		}*/

		// resync the camera position
		syncCameraPos();
	}
};