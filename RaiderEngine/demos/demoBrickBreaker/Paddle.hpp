#pragma once
#include "GameObject2D.hpp"
#include "stdafx.h"


class Paddle : public GameObject2D {
public:
	float speed = 800;
	Paddle(glm::vec2 position) : GameObject2D(position, 0, glm::vec2(1), glm::vec3(1), "paddle.png") {}

	void restart();

	void update() override;
};
