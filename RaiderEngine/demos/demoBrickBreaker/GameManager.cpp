#include "stdafx.h"
#if (COMPILE_DEMO == DEMO_BRICK_BREAKER)
#include "GameManager.hpp"
#include "Brick.hpp"
#include "settings.hpp"
#include "FpsDisplay.hpp"
#include "input.hpp"
#include "timing.hpp"

GameManager::GameManager(std::string fontName, int fontSize) : TextObject("Press Enter to Start", SCR_WIDTH / 2.f, SCR_HEIGHT / 2.f, glm::vec3(1, 1, 1), fontName, fontSize, true) {
	paddle = (Paddle*)addGameObject2D(new Paddle(glm::vec2(0)));
	ball = (Ball*)addGameObject2D(new Ball(glm::vec2(0)));
	addTextObject(new FpsDisplay(6, 6, glm::vec3(1, 1, 1), "Inter-Regular", 18));
	addTextObject(new TextObject("Score: 0", 6, 30, glm::vec3(.8f, .2f, .5f), "Inter-Regular", 18));
	addTextObject(new TextObject("Level: 1", 6, 54, glm::vec3(.6f, .4f, .5f), "Inter-Regular", 18));
	restartGame();
}

void GameManager::restartGame(bool clearScore) {
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

void GameManager::incrementLevel() {
	++level;
	restartGame(false);
}

void GameManager::update() {
	if (paused && keyPressed("restart")) {
		paused = false;
		restartGame();
	}
	else if (gameObject2Ds["brick"].size() == 0)
		incrementLevel();
}

void GameManager::draw(Shader s) {
	if (!paused)
		return;
	scale = 1.0f + .25f * static_cast<float>(sin(totalTime));
	TextObject::draw(s);
}
#endif