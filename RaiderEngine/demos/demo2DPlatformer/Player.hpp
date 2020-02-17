#pragma once
#include "stdafx.h"
#include "GameObject2D.hpp"

class Player : public GameObject2D {
public:
	bool facingRight = true, grounded = false;
	Player(glm::vec2 position);
	glm::vec2 vel;
	float accel = .4f, maxVel = 1.f, groundDecel = .4f, airDecel = .2f, gravity = .16f, jumpVel = 2.8f, maxFallVel = 3.f, wallJumpxVel = 3.f, wallJumpyVel = 2.6f, wallSlideVel = .2f;

	bool anyCollisions();
	bool checkGrounded();
	bool tryMove(bool isLeft = true);
	int inWallJumpRange();
	void update() override;
};
