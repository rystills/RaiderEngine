#pragma once
#include "stdafx.h"
#include "GameObject.hpp"

class Cog : public GameObject {
public:
	float elapsedTime = 0;
	float rotSpeed;
	bool counterClockWise;
	bool wallMounted;
	Cog(glm::vec3 position, glm::vec3 rotationEA, glm::vec3 scale, float rotSpeed, bool counterClockWise, bool wallMounted = true) : rotSpeed(rotSpeed), counterClockWise(counterClockWise), wallMounted(wallMounted), GameObject(position, rotationEA, scale, "cog", wallMounted ? 2 : 0) {}
	void update(float deltaTime) override {
		GameObject::update(deltaTime);
		elapsedTime += deltaTime;
		if (wallMounted) {
			// rotation for wall-mounted cogs is purely graphical; no need to get the physics engine involved
			rotation = glm::rotate(rotation, elapsedTime / 2 * rotSpeed*(counterClockWise ? -1 : 1), glm::vec3(0, 1, 0));
		}
	}

	std::string getDisplayString() override {
		return wallMounted ? "Seems like these cogs are still running...somehow." : "A rusty old cog. Should still be able to function.";
	}
};