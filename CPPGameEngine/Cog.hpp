#ifndef COG_H
#define COG_H

#include "GameObject.hpp"

class Cog : public GameObject {
public:
	float elapsedTime = 0;
	float rotSpeed;
	bool counterClockWise;
	Cog(glm::vec3 position, glm::vec3 rotationEA, glm::vec3 scale, float rotSpeed, bool counterClockWise) : rotSpeed(rotSpeed), counterClockWise(counterClockWise), GameObject(position, rotationEA, scale, "cog") { }
	
	void update(float deltaTime) override {
		elapsedTime += deltaTime;
		GameObject::update(deltaTime);
		// note: because we use a static mesh, setting the rotation in bullet won't affect the simulation; it's purely visual
		/*btTransform trans;
		body->getMotionState()->getWorldTransform(trans);
		float z, y, x;
		trans.getRotation().getEulerZYX(z, y, x);
		rotation = glm::rotate(rotation, elapsedTime / 2 * rotSpeed*(counterClockWise ? -1 : 1), glm::vec3(0, 1, 0));*/
	}
};

#endif