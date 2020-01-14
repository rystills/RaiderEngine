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
	void update() override;

	std::string getDisplayString();
};