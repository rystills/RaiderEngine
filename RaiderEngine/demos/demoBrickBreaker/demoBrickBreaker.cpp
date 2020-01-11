#include "stdafx.h"
// engine includes (import order matters here, at least for the time being)
#include "terminalColors.hpp"
#include "timing.hpp"
#include "physics.hpp"
#include "settings.hpp"
#include "graphics.hpp"
#include "shader.hpp"
#include "FpsDisplay.hpp"
#include "audio.hpp"
#include "GameObject2D.hpp"

void restartGame(bool clearScore);
int score = 0, level = 1;
bool paused = true;
#define levelMultiplier (1 + level * .2f)

class Paddle : public GameObject2D {
public:
	float speed = 800;
	Paddle(glm::vec2 position) : GameObject2D(position, 0, glm::vec2(1), glm::vec3(1), "paddle.png") {}

	void restart() {
		setCenter(glm::vec2(SCR_WIDTH / 2, SCR_HEIGHT - 80));
	}

	void update(float deltaTime) override {
		if (paused)
			return;
		position.x += (keyStates[GLFW_KEY_D][held] - keyStates[GLFW_KEY_A][held]) * speed * deltaTime * levelMultiplier;
		setCenter(glm::vec2(std::fmax(0, std::fmin(SCR_WIDTH, center().x)), center().y));
	}
};

Paddle* paddle;

class Ball : public GameObject2D {
public:
	float speed = 700;
	Ball(glm::vec2 position) : GameObject2D(position, 0, glm::vec2(1), glm::vec3(1), "ball.png") {}

	void restart() {
		setCenter(glm::vec2(SCR_WIDTH / 2, SCR_HEIGHT / 2 + 200));
		rotation = -glm::quarter_pi<float>();
	}

	bool collision(GameObject2D* o) {
		// simple AABB collision check
		return (position.x + sprite.width > o->position.x&& position.x < o->position.x + o->sprite.width &&
			position.y + sprite.height > o->position.y&& position.y < o->position.y + o->sprite.height);
	}

	void bounce(bool isVert) {
		// bounce by flipping either the x or y component of the rotation vector
		rotation = std::atan2(sin(rotation) * (isVert ? -1 : 1), cos(rotation) * (isVert ? 1 : -1));
	}

	void update(float deltaTime) override {
		if (paused)
			return;
		// move forward
		position.x += cos(rotation) * speed * deltaTime * levelMultiplier;
		position.y += sin(rotation) * speed * deltaTime * levelMultiplier;

		// bounce off of screen borders, restarting the game on contact with the bottom of the screen
		if (center().y > SCR_HEIGHT) {
			paused = true;
			return;
		}
		if (center().y < 0) {
			setCenter(glm::vec2(center().x, 0));
			bounce(1);
		}

		if (center().x > SCR_WIDTH) {
			setCenter(glm::vec2(SCR_WIDTH, center().y));
			bounce(0);
		}
		else if (center().x < 0) {
			setCenter(glm::vec2(0, center().y));
			bounce(0);
		}

		// bounce off of bricks, breaking them in the process
		for (int i = 0; i < gameObject2Ds["brick"].size(); ++i) {
			if (collision(&*gameObject2Ds["brick"][i])) {
				GameObject2D o = *gameObject2Ds["brick"][i];
				// if we are moving away from the brick on one axis, the correct bounce must be on the other axis
				if (center().x > o.center().x && cos(rotation) > 0 || center().x < o.center().x && cos(rotation) < 0)
					bounce(1);
				else if (center().y > o.center().y && sin(rotation) > 0 || center().y < o.center().y && sin(rotation) < 0)
					bounce(0);
				else {
					// either axis could be a valid bounce, so choose the axis on which we are penetrating the brick the least
					float xPen = std::fmin(std::abs(position.x + sprite.width - o.position.x), std::abs(o.position.x + o.sprite.width - position.x));
					float yPen = std::fmin(std::abs(position.y + sprite.height - o.position.y), std::abs(o.position.y + o.sprite.height - position.y));
					bounce(xPen > yPen);
				}
				// destroy the brick and update our score
				removeGameObject2D("brick", i);
				score += 100;
				textObjects[1]->text = "Score: " + std::to_string(score);
				break;
			}
		}

		// bounce off of the paddle, picking a new direction based off of our horizontal distance from the paddle's center
		if (collision(paddle)) {
			position.y = paddle->position.y - sprite.height;
			float xOff = center().x - paddle->center().x;
			rotation = -glm::half_pi<float>() + (3 * glm::pi<float>() / 8) * (xOff / (sprite.width/2 + paddle->sprite.width/2));
		}
	}
};

Ball* ball;

class Brick : public GameObject2D {
public:
	Brick(glm::vec2 position, glm::vec3 color) : GameObject2D(position, 0, glm::vec2(1), color, "brick.png") {};
};

void restartGame(bool clearScore = true) {
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

int main() {
	// directories
	setTextureDir("demos/demoBrickBreaker/images");
	setSoundDir("demos/demoBrickBreaker/sounds");
	setFontDir("demos/demo3dCarousel/fonts");

	// initialization
	window = initGraphics();
	initAudio();
	freetypeLoadFont("Inter-Regular", 18);
	freetypeLoadFont("Inter-Regular", 24);
	
	setVsync(true);
	clearColor = glm::vec4(.8f, .8f, 1, 1);
	glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
	paddle = (Paddle*)addGameObject2D(new Paddle(glm::vec2(0)));
	ball = (Ball*)addGameObject2D(new Ball(glm::vec2(0)));
	addTextObject(new FpsDisplay(6, 6, glm::vec3(1, 1, 1), 18));
	addTextObject(new TextObject("Score: 0", 6, 30, glm::vec3(.8f, .2f, .5f), 18));
	addTextObject(new TextObject("Level: 1", 6, 54, glm::vec3(.6f, .4f, .5f), 18));
	restartGame();

	while (!glfwWindowShouldClose(window)) {
		// update frame
		updateTime();
		resetSingleFrameInput();
		glfwPollEvents();

		// update objects
		if (paused && keyStates[GLFW_KEY_ENTER][pressed]) {
			paused = false;
			restartGame();
		}
		else if (gameObject2Ds["brick"].size() == 0)
			incrementLevel();
		updateObjects();

		// render
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		render2D();
		if (paused)
			renderText("Inter-Regular", 24, *shaders["textShader"], "Press Enter to Start", SCR_WIDTH / 2, SCR_HEIGHT / 2, 1.0f, glm::vec3(1, 1, 1), true);
		glfwSwapBuffers(window);
		// set the close flag if the player presses the escape key
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);
	}
	glfwTerminate();
}