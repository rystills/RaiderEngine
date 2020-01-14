#include "stdafx.h"
#include "GameObject.hpp"
#include "timing.hpp"
#include "Cog.hpp"
void Cog::update() {
	GameObject::update();
	elapsedTime += deltaTime;
	if (wallMounted) {
		// rotation for wall-mounted cogs is purely graphical; no need to get the physics engine involved
		rotation = glm::rotate(rotation, elapsedTime / 2 * rotSpeed * (counterClockWise ? -1 : 1), glm::vec3(0, 1, 0));
	}
}

std::string Cog::getDisplayString() {
	return wallMounted ? "Seems like these cogs are still running...somehow." : "A rusty old cog. Should still be able to function.";
}