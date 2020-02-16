#pragma once
#include "stdafx.h"
#include "GameObject2D.hpp"

class Player : public GameObject2D {
public:
	bool facingRight = true, grounded = false;
	Player(glm::vec2 position);
	glm::vec2 vel;
	float accel = .4f, maxVel = 1.f, decel = .4f, gravity = .2f, jumpVel = 3.f, maxFallVel = 4.f;

	bool anyCollisions();
	bool checkGrounded();
	bool tryMove(bool isLeft = true);
	void update() override;
};
