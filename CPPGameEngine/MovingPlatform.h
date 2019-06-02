#ifndef MOVING_PLATFORM_H
#define MOVING_PLATFORM_H

#include "GameObject.h"

class MovingPlatform : public GameObject
{
public:
	float elapsedTime = 0;
	// TODO: remove this constraint from the world and delete it on death
	btGeneric6DofConstraint* moveConstraint;
	MovingPlatform(glm::vec3 position, glm::vec3 rotationEA, glm::vec3 scale) :
		GameObject(position, rotationEA, scale, "movingPlatform", false,false) {
		// create a constraint at our starting position, and move that up and down
		btTransform tr;
		tr.setIdentity();
		tr.setOrigin(btVector3(position.x, position.y, position.z));
		moveConstraint = new btGeneric6DofConstraint(*body, tr, true);
		moveConstraint->setLinearLowerLimit(btVector3(0, 0, 0));
		moveConstraint->setLinearUpperLimit(btVector3(0, 0, 0));
		moveConstraint->setAngularLowerLimit(btVector3(0, 0, 0));
		moveConstraint->setAngularUpperLimit(btVector3(0, 0, 0));
		bulletData.dynamicsWorld->addConstraint(moveConstraint, true);
		for (int i = 0; i < 6; ++i) {
			// CFM (constraint force mixing): increase this to make the constraint softer
			// ERP (error reduction parameter): increase this to fix a greater proportion of the accumulated error each step
			moveConstraint->setParam(BT_CONSTRAINT_STOP_CFM, 0.8f, i);
			moveConstraint->setParam(BT_CONSTRAINT_STOP_ERP, 0.5f, i);
		}
		body->setGravity(btVector3(0, 0, 0));
	}
	
	void update(float deltaTime) override {
		// update transform position based on time
		elapsedTime += deltaTime;
		body->activate(true);		
		GameObject::update(deltaTime);
		btVector3 origin = moveConstraint->getFrameOffsetA().getOrigin();
		moveConstraint->getFrameOffsetA().setOrigin(btVector3(origin.getX(),origin.getY()+sin(elapsedTime)*.01f, origin.getZ()));
	}
};

#endif