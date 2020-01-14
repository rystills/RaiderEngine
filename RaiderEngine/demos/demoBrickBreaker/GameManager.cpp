#include "stdafx.h"
#include "GameManager.hpp"
#include "Brick.hpp"
#include "settings.hpp"

void restartGame(bool clearScore) {
	// reset objects
	if (clearScore) {
		score = 0;
		textObjects[1]->text = "Score: " + std::to_string(score);
		level = 1;
	}
	textObjects[2]->text = "Level: " + std::to_string(level);
	paddle->restart();
	ball->restart();

	// clear and rebuild bricks
	gameObject2Ds["brick"].clear();
	for (int i = 0; i < 20; ++i)
		for (int r = 0; r < 4; ++r)
			addGameObject2D(new Brick(glm::vec2(i * 64, r * 32), glm::vec3(i / 20.f, r / 4.f, .5f)));
}

void incrementLevel() {
	++level;
	restartGame(false);
}