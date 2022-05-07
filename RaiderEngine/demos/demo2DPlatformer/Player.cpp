#include "stdafx.h"
#if (COMPILE_DEMO == DEMO_2D_PLATFORMER)
#include "player.hpp"
#include "input.hpp"
#include "timing.hpp"
#include "settings.hpp"
#include "Collider2DRectangle.hpp"
#include <constants.hpp>

Player::Player(glm::vec2 position) : GameObject2D(position, 0, glm::vec2(1), Color::white, "player.png") {
	collider = colliders.contains("player") ? colliders["player"].get() : addCollider2D("player", new Collider2DRectangle(4,4));
	vel = glm::vec2(0);
}

bool Player::checkGrounded() {
	if (vel.y >= 0) {
		translate(glm::vec2(0, 1));
		if (anyCollisions()) {
			// move onto the pixel grid vertically before attempting collision resolution
			setPos(position.x, (int)position.y);
			while (anyCollisions())
				translate(0, -1);
			return true;
		}
		translate(glm::vec2(0, -1));
	}
	return false;
}

bool Player::anyCollisions() {
	for (auto&& kv : gameObject2Ds)
		for (unsigned int i = 0; i < kv.second.size(); ++i)
			if (&*kv.second[i] != (GameObject2D*)this && collidesWith(&*kv.second[i]))
				return true;
	for (auto& t : tilemaps)
		if (collidesWith(&*t))
			return true;
	return false;
}

bool Player::tryMove(bool isLeft) {
	// TODO: perform movement in spriteSize subsets to avoid phasing at high speeds (same for grounded check)
	translate(isLeft ? vel.x : 0, isLeft ? 0 : vel.y);
	if (!anyCollisions())
		return false;
	// move onto the pixel grid before attempting collision resolution
	setPos(glm::vec2(isLeft ? (int)position.x : position.x, isLeft ? position.y : (int)position.y));
	while (anyCollisions())
		translate(isLeft ? signbit(vel.x) * 2 - 1 : 0, isLeft ? 0 : signbit(vel.y) * 2 - 1);
	return true;
}

int Player::inWallJumpRange() {
	// TODO: if surrounded by walls on both sides, return the side with the closer wall
	for (int i = -1; i <= 1; i += 2) {
		// check 4 pixels away
		translate(glm::vec2(i * 4, 0));
		bool col = anyCollisions();
		translate(glm::vec2(i * -4, 0));
		if (col)
			return i == -1 ? 1 : 2;
	}
	return 0;
}

int Player::wallClimbing() {
	if (keyHeld("climb")) {
		translate(facingRight * 2 - 1, 0);
		if (anyCollisions()) {
			// TODO: abstract collision resolution to its own method
			// move onto the pixel grid horizontally before attempting collision resolution
			setPos((int)position.x, position.y);
			while (anyCollisions())
				translate(-(facingRight * 2 - 1), 0);
			return true;
		}
		translate(-(facingRight * 2 - 1), 0);
	}
	return false;
}

void Player::update() {
	// grounded check
	grounded = checkGrounded();
	float decel = grounded ? groundDecel : airDecel;

	// input -> horizontal delta velocity
	glm::vec2 dVel((keyHeld("mvRight") - keyHeld("mvLeft")) * accel, 0);
	// horizontal deceleration
	if (dVel.x == 0 && vel.x != 0)
		dVel.x = abs(vel.x) <= decel ? -vel.x : decel * (signbit(vel.x)*2-1);
	vel.x = (std::max(-maxVel, std::min(maxVel, vel.x + dVel.x)));
	// facing direction check
	if (vel.x != 0) {
		facingRight = signbit(-vel.x);
		setScale(facingRight * 2 - 1, scaleVal.y);
	}
	// vertical delta velocity
	vel.y = grounded ? 0 : std::min(vel.y+gravity, maxFallVel);
	// jump check
	if (keyPressed("jump")) {
		// grounded jump
		if (grounded)
			vel.y = -jumpVel;
		// wall jump
		else if (int wallDir = inWallJumpRange())
			vel = glm::vec2(wallDir == 1 ? wallJumpxVel : -wallJumpxVel, -wallJumpyVel);
	}
	// move & resolve horizontal collisions
	if (tryMove(true) || (wallClimbing()))
		// allow wall sliding and grabbing
		vel = glm::vec2(0,std::min(vel.y,keyHeld("climb") ? (keyHeld("mvUp") - keyHeld("mvDown")) * -wallClimbVel : wallSlideVel));
	// move & resolve vertical collisions
	if (tryMove(false))
		vel.y = 0;
}
#endif