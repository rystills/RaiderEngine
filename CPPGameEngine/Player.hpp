#ifndef PLAYER_H
#define PLAYER_H
#include "camera.hpp"
#include <BulletDynamics/Character/btKinematicCharacterController.h>
#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>

class Player {
public:
	Camera camera;
	btKinematicCharacterController* controller;
	btPairCachingGhostObject* ghostObject;
	btCapsuleShape* convexShape;
	float radius = .5f;
	float height = 1.6f;

	Player() : camera(glm::vec3(0)) { }

	/*
	initialize the player once Bullet initialization has been completed
	*/
	void init() {
		// transform init
		btTransform startTransform;
		startTransform.setIdentity();
		startTransform.setOrigin(btVector3(0, 2, 0));

		// shape init
		convexShape = new btCapsuleShape(radius, height);
		bulletData.collisionShapes.push_back(convexShape);

		// ghost init
		ghostObject = new btPairCachingGhostObject();
		ghostObject->setWorldTransform(startTransform);
		ghostObject->setCollisionShape(convexShape);
		ghostObject->setCollisionFlags(btCollisionObject::CF_CHARACTER_OBJECT);

		// controller init
		controller = new btKinematicCharacterController(ghostObject, convexShape, 1);
		controller->setGravity(bulletData.dynamicsWorld->getGravity());
		
		// add ghost to world
		bulletData.dynamicsWorld->addCollisionObject(ghostObject, btBroadphaseProxy::CharacterFilter, btBroadphaseProxy::StaticFilter | btBroadphaseProxy::DefaultFilter);
		bulletData.dynamicsWorld->addAction(controller);
		bulletData.overlappingPairCache->getOverlappingPairCache()->setInternalGhostPairCallback(new btGhostPairCallback());
	}

	/*
	set the player's position in world space
	@param pos: the position at which to place the player
	*/
	void setPos(glm::vec3 pos) {
		btTransform newTrans;
		newTrans.setIdentity();
		newTrans.setOrigin(btVector3(pos.x, pos.y, pos.z));
		ghostObject->setWorldTransform(newTrans);
		syncCameraPos();
	}

	/*
	sync the position of the camera with the player's current position
	*/
	void syncCameraPos() {
		btVector3 pos = ghostObject->getWorldTransform().getOrigin();
		camera.Position.x = pos.getX();
		// camera height should be set to the top of the capsule minus the approximate distance from the top of the head to the eyes
		camera.Position.y = pos.getY() + (height / 2 - .12f);
		camera.Position.z = pos.getZ();
	}

	void update(GLFWwindow *window, float deltaTime) {
		// process input
		controller->setWalkDirection(btVector3(0, 0, 0));
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);

		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
			controller->setWalkDirection(btVector3(camera.Front.x*.01f, camera.Front.y*.01f, camera.Front.z*.01f));
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
			controller->setWalkDirection(btVector3(-camera.Front.x*.01f, -camera.Front.y*.01f, -camera.Front.z*.01f));
		}

		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
			controller->setWalkDirection(btVector3(-camera.Right.x*.01f, -camera.Right.y*.01f, -camera.Right.z*.01f));
		}
		
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
			controller->setWalkDirection(btVector3(camera.Right.x*.01f, camera.Right.y*.01f, camera.Right.z*.01f));
		}

		// resync the camera position
		syncCameraPos();
	}
};
#endif