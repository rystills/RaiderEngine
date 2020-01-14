#pragma once
#include "GameObject2D.hpp"
class Brick : public GameObject2D {
public:
	Brick(glm::vec2 position, glm::vec3 color) : GameObject2D(position, 0, glm::vec2(1), color, "brick.png") {};
};