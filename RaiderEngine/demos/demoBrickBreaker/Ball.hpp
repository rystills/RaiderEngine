#pragma once
#include "stdafx.h"
#include "GameObject2D.hpp"

class Ball : public GameObject2D {
public:
	float speed = 700;
	Ball(glm::vec2 position) : GameObject2D(position, 0, glm::vec2(1), glm::vec3(1), "ball.png") {}

	void restart();

	bool collision(GameObject2D* o);

	void bounce(bool isVert);

	void update() override;
};