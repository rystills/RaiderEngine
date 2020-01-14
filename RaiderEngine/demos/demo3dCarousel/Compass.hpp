#pragma once
#include "stdafx.h"
#include "GameObject2D.hpp"

class Compass : public GameObject2D {
public:
	Compass(glm::vec2 position, float rotation, glm::vec2 scale, glm::vec3 color, std::string spriteName) : GameObject2D(position, rotation, scale, color, spriteName) {}
	void update() override;
};