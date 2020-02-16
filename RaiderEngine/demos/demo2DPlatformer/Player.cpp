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
			if (&*kv.second[i] != (GameObject2D*)this && kv.second[i]->collider)
				if (collidesWith(&*kv.second[i]))
					return true;
	for (auto& t : tilemaps) {
		// check collisions with all tiles that lie within the square containing our bounding radius
		float normalX = center.x - t->pos.x, normalY = center.y - t->pos.y;
		int gridxMin = std::max(0, static_cast<int>((normalX - collider->boundingRadius) / t->gridSize)),
			gridxMax = std::max(0, static_cast<int>((normalX + collider->boundingRadius) / t->gridSize)),
			gridyMin = std::max(0, static_cast<int>((normalY - collider->boundingRadius) / t->gridSize)),
			gridyMax = std::max(0, static_cast<int>((normalY + collider->boundingRadius) / t->gridSize));
		
		// draw indicators on all tiles that we check for collisions
		if (debugDraw)
			for (unsigned int i = gridxMin; i <= gridxMax && i < t->mapSize.x; ++i)
				for (unsigned int r = gridyMin; r <= gridyMax && r < t->mapSize.y; ++r) {
					queueDrawLine(glm::vec3(t->pos.x + i * t->gridSize + t->gridSize * .5f - 3, t->pos.y + r * t->gridSize + t->gridSize * .5f, 0), glm::vec3(t->pos.x + i * t->gridSize + t->gridSize * .5f + 3, t->pos.y + r * t->gridSize + t->gridSize * .5f, 0), Color::white);
					queueDrawLine(glm::vec3(t->pos.x + i * t->gridSize + t->gridSize * .5f, t->pos.y + r * t->gridSize + t->gridSize * .5f - 3, 0), glm::vec3(t->pos.x + i * t->gridSize + t->gridSize * .5f, t->pos.y + r * t->gridSize + t->gridSize * .5f + 3, 0), Color::white);
				}
		
		for (unsigned int i = gridxMin; i <= gridxMax && i < t->mapSize.x; ++i)
			for (unsigned int r = gridyMin; r <= gridyMax && r < t->mapSize.y; ++r) {
				Collider2D* tcol = t->tileColliders[t->map[i][r]];
				if (tcol != NULL && collider->collision(center, rotation, tcol, glm::vec2(t->pos.x + i * t->gridSize + t->gridSize*.5f, t->pos.y + r * t->gridSize + t->gridSize*.5f), 0))
					return true;
			}
	}
		
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