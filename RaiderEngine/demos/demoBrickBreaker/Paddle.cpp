#include "stdafx.h"
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
	setCenter(glm::vec2(SCR_WIDTH / 2, SCR_HEIGHT - 80));
}

void Paddle::update() {
	if (paused)
		return;
	position.x += (keyStates[GLFW_KEY_D][held] - keyStates[GLFW_KEY_A][held]) * speed * deltaTime * levelMultiplier;
	setCenter(glm::vec2(std::fmax(0, std::fmin(SCR_WIDTH, center().x)), center().y));
}