#include "stdafx.h"
#if (COMPILE_DEMO == DEMO_2D_PLATFORMER)
#include "player.hpp"
#include "input.hpp"
#include "timing.hpp"
#include "settings.hpp"
#include "Collider2DRectangle.hpp"

Player::Player(glm::vec2 position) : GameObject2D(position, 0, glm::vec2(1), Color::white, "player.png") {
	collider = colliders.contains("player") ? colliders["player"].get() : addCollider2D("player", new Collider2DRectangle(4,4));
	vel = glm::vec2(0);
}

bool Player::checkGrounded() {
	if (vel.y < 0)
		return false;
	translate(glm::vec2(0, 1));
	bool groundCol = anyCollisions();
	translate(glm::vec2(0, -1));
	return groundCol;
}

bool Player::anyCollisions() {
	for (auto&& kv : gameObject2Ds)
		for (unsigned int i = 0; i < kv.second.size(); ++i)
			if (&*kv.second[i] != (GameObject2D*)this && kv.second[i]->collider)
				if (collidesWith(&*kv.second[i]))
					return true;
	for (auto& t : tilemaps) {
		// perform precise collision checking with all tiles that lie inside our bounding radius, with a buffer margin up to the tilemap's gridSize
		int gridx = (center.x - t->pos.x) / t->gridSize;
		int gridy = (center.y - t->pos.y) / t->gridSize;
		int gridxMin = std::max(static_cast<int>(gridx - ceil((collider->boundingRadius+t->gridSize*.5f) / t->gridSize)),0), gridxMax = gridx + ceil((collider->boundingRadius+t->gridSize*.5f) / t->gridSize);
		int gridyMin = std::max(static_cast<int>(gridy - ceil((collider->boundingRadius+t->gridSize*.5f) / t->gridSize)),0), gridyMax = gridy + ceil((collider->boundingRadius+t->gridSize*.5f) / t->gridSize);
		for (unsigned int i = gridxMin; i <= gridxMax && i < t->mapSize.x; ++i)
			for (unsigned int r = gridyMin; r <= gridyMax && r < t->mapSize.y; ++r) {
				Collider2D* tcol = t->tileColliders[t->map[i][r]];
				if (tcol != NULL && collider->collision(center, rotation, tcol, glm::vec2(t->pos.x + i * t->gridSize, t->pos.y + r * t->gridSize), 0))
					return true;
			}
	}
		
	return false;
}

bool Player::tryMove(bool isLeft) {
	translate(isLeft ? vel.x : 0, isLeft ? 0 : vel.y);
	if (!anyCollisions())
		return false;
	do translate(isLeft ? signbit(vel.x) * 2 - 1 : 0, isLeft ? 0 : signbit(vel.y) * 2 - 1); while (anyCollisions());
	return true;
}

void Player::update() {
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
	vel.y = (grounded = checkGrounded()) ? 0 : std::min(vel.y+gravity, maxFallVel);
	// jump check
	if (grounded && keyPressed("jump"))
		vel.y = -jumpVel;
	
	// move & resolve horizontal collisions
	if (tryMove(true))
		vel.x = 0;
	// move & resolve vertical collisions
	if (tryMove(false))
		vel.y = 0;
}

#endif