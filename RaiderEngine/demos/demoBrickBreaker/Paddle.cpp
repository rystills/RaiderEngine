#include "stdafx.h"
#if (COMPILE_DEMO == DEMO_BRICK_BREAKER)
#include "Paddle.hpp"
#include "input.hpp"
#include "timing.hpp"
#include "settings.hpp"
#include "GameManager.hpp"
#include "Collider2DRectangle.hpp"

Paddle::Paddle(glm::vec2 position) : GameObject2D(position, 0, glm::vec2(1), glm::vec3(1), "paddle.png") {
	collider = colliders.contains("paddle") ? colliders["paddle"].get() : addCollider2D("paddle", new Collider2DRectangle(32, 4));
}

void Paddle::restart() {
	setCenter(TARGET_WIDTH / 2.f, TARGET_HEIGHT - 80.f);
}

void Paddle::update() {
	if (paused)
		return;
	translate((keyHeld("mvRight") - keyHeld("mvLeft")) * speed * deltaTime * levelMultiplier,0);
	setCenter(std::fmaxf(0, std::fminf(static_cast<float>(TARGET_WIDTH), center.x)), center.y);
}
#endif