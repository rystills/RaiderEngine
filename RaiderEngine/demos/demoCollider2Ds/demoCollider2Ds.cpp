#include "stdafx.h"
#include "GameObject2D.hpp"
#include "settings.hpp"
#include "timing.hpp"
#include "audio.hpp"
#include "FpsDisplay.hpp"
#include "input.hpp"
#include "Collider2DRectangle.hpp"

int main() {
	// directories
	setFontDir("demos/shared/fonts");

	// initialization
	window = initGraphics();
	initAudio();
	freetypeLoadFont("Inter-Regular", 18);

	setVsync(false);
	debugDraw = true;
	clearColor = glm::vec4(.6f, 1.f, .4f, 1);
	glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
	addTextObject(new FpsDisplay(6, 6, glm::vec3(0, 0, 0), 18));
	addTextObject(new TextObject("Colliding ? ", 6, 30, glm::vec3(.8f, .2f, .5f), 18));
	Collider2DRectangle* rectCol = new Collider2DRectangle(100,100);
	addGameObject2D(new GameObject2D(glm::vec2(200, 200), 0, glm::vec2(1), glm::vec3(1), "", false, 0, rectCol));
	addGameObject2D(new GameObject2D(glm::vec2(350, 225), 0, glm::vec2(1), glm::vec3(1), "", false, 0, rectCol));

	while (!glfwWindowShouldClose(window)) {
		// update frame
		updateTime();
		resetSingleFrameInput();
		glfwPollEvents();

		updateObjects();

		if (keyStates[GLFW_KEY_A][held])
			gameObject2Ds[""][0]->position.x -= 200 * deltaTime;
		if (keyStates[GLFW_KEY_D][held])
			gameObject2Ds[""][0]->position.x += 200 * deltaTime;
		if (keyStates[GLFW_KEY_W][held])
			gameObject2Ds[""][0]->position.y -= 200 * deltaTime;
		if (keyStates[GLFW_KEY_S][held])
			gameObject2Ds[""][0]->position.y += 200 * deltaTime;

		bool colA = gameObject2Ds[""][0]->collider->collision(gameObject2Ds[""][0]->position, gameObject2Ds[""][0]->rotation, gameObject2Ds[""][1]->collider,
			gameObject2Ds[""][1]->position, gameObject2Ds[""][1]->rotation);
		bool colB = gameObject2Ds[""][1]->collider->collision(gameObject2Ds[""][1]->position, gameObject2Ds[""][1]->rotation,
			gameObject2Ds[""][0]->collider, gameObject2Ds[""][0]->position, gameObject2Ds[""][0]->rotation);
		textObjects[1]->text = "Colliding ? " + (colA ? std::string("true") : std::string("false")) + ", " + (colB ? std::string("true") : std::string("false"));

		// render
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		renderLines2D();
		render2D();
		glfwSwapBuffers(window);
		// set the close flag if the player presses the escape key
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);
	}
	glfwTerminate();
}