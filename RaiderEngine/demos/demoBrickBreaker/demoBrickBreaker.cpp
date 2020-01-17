#include "stdafx.h"
#include "GameObject2D.hpp"
#include "settings.hpp"
#include "timing.hpp"
#include "audio.hpp"
#include "FpsDisplay.hpp"
#include "input.hpp"
#include "Paddle.hpp"
#include "Ball.hpp"
#include "GameManager.hpp"
#include "Collider2DCircle.hpp"

int main() {
	// directories
	setTextureDir("demos/demoBrickBreaker/images");
	setSoundDir("demos/demoBrickBreaker/sounds");
	setFontDir("demos/shared/fonts");

	// initialization
	window = initGraphics();
	initAudio();
	freetypeLoadFont("Inter-Regular", 18);
	freetypeLoadFont("Inter-Regular", 48);
	
	setVsync(false);
	clearColor = glm::vec4(.8f, .8f, 1, 1);
	glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
	paddle = (Paddle*)addGameObject2D(new Paddle(glm::vec2(0)));
	ball = (Ball*)addGameObject2D(new Ball(glm::vec2(0)));
	addTextObject(new FpsDisplay(6, 6, glm::vec3(1, 1, 1), "Inter-Regular", 18));
	addTextObject(new TextObject("Score: 0", 6, 30, glm::vec3(.8f, .2f, .5f), "Inter-Regular", 18));
	addTextObject(new TextObject("Level: 1", 6, 54, glm::vec3(.6f, .4f, .5f), "Inter-Regular", 18));
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
			renderText("Inter-Regular", 48, *shaders["textShader"], "Press Enter to Start", SCR_WIDTH / 2, SCR_HEIGHT / 2, 1.0f, glm::vec3(1, 1, 1), true);
		glfwSwapBuffers(window);
		// set the close flag if the player presses the escape key
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);
	}
	glfwTerminate();
}