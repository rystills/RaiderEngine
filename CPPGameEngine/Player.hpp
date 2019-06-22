#pragma once
#include "stdafx.h"
#include "camera.hpp"

void applyTransformCallbackRedirect(const NewtonBody* body, const dFloat* matrix, int threadIndex);

#define PLAYER_RADIUS .5f
#define PLAYER_HEIGHT  1.9f
#define PLAYER_MASS  80.0f
#define PLAYER_WALK_SPEED  5.0f
#define PLAYER_RUN_SPEED  8.0f
#define PLAYER_JUMP_SPEED  6.0f
#define PLAYER_THIRD_PERSON_VIEW_DIST  8.0f
bool canJump = true;
class BasicPlayerControllerManager : public dCustomPlayerControllerManager
{
public:
	BasicPlayerControllerManager(NewtonWorld* const world) : dCustomPlayerControllerManager(world), m_player(NULL) {}

	~BasicPlayerControllerManager() {}

	void SetAsPlayer(dCustomPlayerController* const controller) {
		m_player = controller;
	}
	
	dCustomPlayerController* CreatePlayer(const dMatrix& location, dFloat height, dFloat radius, dFloat mass) {
		// set the play coordinate system
		dMatrix localAxis(dGetIdentityMatrix());

		//up is first vector
		localAxis[0] = dVector(0.0, 1.0f, 0.0f, 0.0f);
		// up is the second vector
		localAxis[1] = dVector(1.0, 0.0f, 0.0f, 0.0f);
		// size if the cross product
		localAxis[2] = localAxis[0].CrossProduct(localAxis[1]);

		// make a play controller with default values.
		dCustomPlayerController* const controller = CreateController(location, localAxis, mass, radius, height, height / 3.0f);

		// Test Local Matrix manipulations
		//controller->SetFrame(dRollMatrix(60.0f * dDegreeToRad) * controller->GetFrame());

		// get body from player, and set some parameter
		NewtonBody* const body = controller->GetBody();

		// create the visual mesh from the player collision shape
		NewtonCollision* const collision = NewtonBodyGetCollision(body);

		// set the transform callback
		NewtonBodySetTransformCallback(body, applyTransformCallbackRedirect);

		// save player model with the controller
		//controller->SetUserData(playerEntity);

		return controller;
	}

	bool ProccessContact(dCustomPlayerController* const controller, const dVector& position, const dVector& normal, const NewtonBody* const otherbody) const {
		/*
				if (normal.m_y < 0.9f) {
					dMatrix matrix;
					NewtonBodyGetMatrix(controller->GetBody(), &matrix[0][0]);
					dFloat h = (position - matrix.m_posit).DotProduct3(matrix.m_up);
					return (h >= m_stepHigh) ? true : false;
				}
		*/
		return true;
	}

	dFloat ContactFriction(dCustomPlayerController* const controller, const dVector& position, const dVector& normal, int contactId, const NewtonBody* const otherbody) const {
		if (normal.m_y < 0.9f) {
			// steep slope are friction less
			return 0.0f;
		}
		else {
			canJump = true;
			// TODO: apply different friction values to different contactIDs; generally ground friction should stay quite high, but it may be lowered on ice / oil / etc.. 
			return 10;
			//NewtonCollision* const collision = NewtonBodyGetCollision(otherbody);
			//int type = NewtonCollisionGetType (collision);
			//if ((type == SERIALIZE_ID_TREE) || (type == SERIALIZE_ID_TREE)) {
			//} else {
			switch (contactId)
			{
			case 1:
				// this the brick wall
				return 0.5f;
			case 2:
				// this the wood floor
				return 1.0f;
			case 3:
				// this the cement floor
				return 2.0f;
				//return 0.2f;
			default:
				// this is everything else
				return 1.0f;
			}
		}
	}

	void ApplyInputs(dCustomPlayerController* const controller);

	// apply gravity 
	virtual void ApplyMove(dCustomPlayerController* const controller, dFloat timestep) {
		// calculate the gravity contribution to the velocity
		dVector gravityImpulse(0.0f, -9.8f * controller->GetMass() * timestep, 0.0f, 0.0f);
		dVector existingImpulse = controller->GetImpulse();
		// when we jump, we ignore gravity and force the y component of our impulse to 200
		dVector totalImpulse((canJump && glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) ? dVector(existingImpulse[0],200,existingImpulse[2]) : (controller->GetImpulse() + gravityImpulse));
		controller->SetImpulse(totalImpulse);
		canJump = false;

		// apply play movement
		ApplyInputs(controller);
	}

	dCustomPlayerController* m_player;
};

class Player {
public:
	Camera camera;
	//kineCon* controller;
	//btPairCachingGhostObject* ghostObject;
	//btCapsuleShape* convexShape;
	/*
	float walkSpeed = .02f;
	float runSpeed = .04f;
	float jumpSpeed = 5;
	float maxStepHeight = .05f;
	*/
	dCustomPlayerController* controller;

	Player() : camera(glm::vec3(0)) { }

	/*void TransformCallback(const NewtonBody* body, const dFloat* matrix, int threadIndex) {
		dFloat force[3] = { 0, -9.8 * PLAYER_MASS, 0 };
		NewtonBodySetForce(body, force);
	}*/

	/*
	initialize the player, creating a new newton controller
	*/
	void init() {
		// configure the starting location
		dMatrix location(dGetIdentityMatrix());
		location.m_posit.m_y = 0;
		BasicPlayerControllerManager* const playerManager = new BasicPlayerControllerManager(world);
		controller = playerManager->CreatePlayer(location, PLAYER_HEIGHT, PLAYER_RADIUS, PLAYER_MASS);
		playerManager->SetAsPlayer(controller);
		// set the user data
		NewtonBodySetUserData(controller->GetBody(), this);
	}

	/*
	set the player's position in world space
	@param pos: the position at which to place the player
	*/
	void setPos(glm::vec3 pos) {
		// TODO: you may need to freeze the world before modifying the player's position; it's probably safe regardless at least when used initially by PlayerSpawn
		NewtonBody* bod = controller->GetBody();
		dMatrix mat;
		NewtonBodyGetMatrix(bod, &mat[0][0]);
		mat.m_posit.m_x = pos.x;
		mat.m_posit.m_y = pos.y;
		mat.m_posit.m_z = pos.z;
		NewtonBodySetMatrix(bod, &mat[0][0]);
	}

	/*
	sync the position of the camera with the player's current position
	*/
	void syncCameraPos(dFloat* pos) {
		camera.Position.x = pos[0];
		// camera height should be set to the top of the capsule minus the approximate distance from the top of the head to the eyes
		//camera.Position.y = pos[1] + (PLAYER_HEIGHT / 2 - .12f);
		camera.Position.y = pos[1] + (PLAYER_HEIGHT - .12f);
		camera.Position.z = pos[2];
		
	}

	void update(GLFWwindow *window, float deltaTime) {
		NewtonBody* const body = controller->GetBody();
		dFloat pos[4];
		NewtonBodyGetPosition(body, pos);
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
		syncCameraPos(pos);

		camera.updateViewProj();
	}
};

void applyTransformCallbackRedirect(const NewtonBody* body, const dFloat* matrix, int threadIndex) {
	// retrieve the corresponding GameObject from the body's user data
	Player* p = (Player*)NewtonBodyGetUserData(body);
	// allow the gameObject to handle applying force
	//p->TransformCallback(body, matrix, threadIndex);
}

void BasicPlayerControllerManager::ApplyInputs(dCustomPlayerController* const controller) {
	if (controller == m_player) {
		// walk/run
		float baseMoveSpeed = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ? PLAYER_RUN_SPEED : PLAYER_WALK_SPEED);
		dFloat forwarSpeed = (int(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) - int(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)) * baseMoveSpeed;
		dFloat strafeSpeed = (int(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) - int(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)) * baseMoveSpeed;
		if (forwarSpeed && strafeSpeed) {
			dFloat invMag = baseMoveSpeed / dSqrt(forwarSpeed * forwarSpeed + strafeSpeed * strafeSpeed);
			forwarSpeed *= invMag;
			strafeSpeed *= invMag;
		}
		controller->SetForwardSpeed(forwarSpeed);
		controller->SetLateralSpeed(strafeSpeed);
		Player* p = (Player*)NewtonBodyGetUserData(controller->GetBody());
		controller->SetHeadingAngle(dAtan2(-p->camera.Front.z, p->camera.Front.x));
	}
}