#include "stdafx.h"
#include "GameObject2D.hpp"
#include "settings.hpp"
#include "timing.hpp"
#include "audio.hpp"
#include "FpsDisplay.hpp"
#include "input.hpp"
#include "Collider2DRectangle.hpp"
#include "Collider2DCircle.hpp"
#include "Collider2DLine.hpp"
#include "Collider2DPolygon.hpp"

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
	Collider2DCircle* circleCol = new Collider2DCircle(100);
	Collider2DLine* lineCol = new Collider2DLine(0,-75,0,75);
	Collider2DPolygon* polyCol = new Collider2DPolygon({ glm::vec2(-75,0), glm::vec2(-40,-50), glm::vec2(40,-50), glm::vec2(75,0), glm::vec2(40, 50), glm::vec2(-40,50) });
	const int numCols = 4;
	Collider2D* cols[numCols] = { rectCol,circleCol, lineCol, polyCol };
	GameObject2D* g1 = addGameObject2D(new GameObject2D(glm::vec2(SCR_WIDTH/2-300, SCR_HEIGHT/2), 0, glm::vec2(1), glm::vec3(1), "", false, 0, rectCol));
	GameObject2D* g2 = addGameObject2D(new GameObject2D(glm::vec2(SCR_WIDTH/2, SCR_HEIGHT/2), 0, glm::vec2(1), glm::vec3(1), "", false, 0, circleCol));
	int g1ColInd = 0, g2ColInd = 1;

	while (!glfwWindowShouldClose(window)) {
		// update frame
		updateTime();
		resetSingleFrameInput();
		glfwPollEvents();

		updateObjects();

		if (keyStates[GLFW_KEY_A][held])
			g1->position.x -= 200 * deltaTime;
		if (keyStates[GLFW_KEY_D][held])
			g1->position.x += 200 * deltaTime;
		if (keyStates[GLFW_KEY_W][held])
			g1->position.y -= 200 * deltaTime;
		if (keyStates[GLFW_KEY_S][held])
			g1->position.y += 200 * deltaTime;
		
		if (keyStates[GLFW_KEY_Q][held])
			g1->rotation -= 3 * deltaTime;
		if (keyStates[GLFW_KEY_E][held])
			g1->rotation += 3 * deltaTime;

		if (keyStates[GLFW_KEY_R][pressed]) {
			g1ColInd = (g1ColInd + 1) % numCols;
			g1->collider = cols[g1ColInd];
		}
		if (keyStates[GLFW_KEY_F][pressed]) {
			g2ColInd = (g2ColInd + 1) % numCols;
			g2->collider = cols[g2ColInd];
		}

		textObjects[1]->text = "Colliding ? " + (g1->collider->collision(g1->position, g1->rotation, g2->collider, g2->position, g2->rotation) ? std::string("true") : std::string("false")) + 
										" | " + (g2->collider->collision(g2->position, g2->rotation, g1->collider, g1->position, g1->rotation) ? std::string("true") : std::string("false"));

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