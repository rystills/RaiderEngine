#include "stdafx.h"
#include "Paddle.hpp"
#include "input.hpp"
#include "timing.hpp"
#include "settings.hpp"
#include "GameManager.hpp"

void Paddle::restart() {
	setCenter(glm::vec2(SCR_WIDTH / 2, SCR_HEIGHT - 80));
}

void Paddle::update() {
	if (paused)
		return;
	position.x += (keyStates[GLFW_KEY_D][held] - keyStates[GLFW_KEY_A][held]) * speed * deltaTime * levelMultiplier;
	setCenter(glm::vec2(std::fmax(0, std::fmin(SCR_WIDTH, center().x)), center().y));
}