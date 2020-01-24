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

	// keybindings
	setKeyBinding("mvLeft", GLFW_KEY_A);
	setKeyBinding("mvRight", GLFW_KEY_D);
	setKeyBinding("mvUp", GLFW_KEY_W);
	setKeyBinding("mvDown", GLFW_KEY_S);
	setKeyBinding("rotCCW", GLFW_KEY_Q);
	setKeyBinding("rotCW", GLFW_KEY_E);
	setKeyBinding("swap1", GLFW_KEY_R);
	setKeyBinding("swap2", GLFW_KEY_F);


	// initialization
	window = initGraphics();
	initAudio();
	freetypeLoadFont("Inter-Regular", 18);

	setVsync(false);
	debugDraw = true;
	clearColor = glm::vec4(.6f, 1.f, .4f, 1);
	glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
	addTextObject(new FpsDisplay(6, 6, glm::vec3(0, 0, 0), "Inter-Regular", 18));
	addTextObject(new TextObject("Colliding ? ", 6, 30, glm::vec3(.8f, .2f, .5f), "Inter-Regular", 18));
	addTextObject(new TextObject("Press WASD to move, Q+E to rotate, and R+F to cycle through object 1 and 2's colliders", 6, 54, glm::vec3(.5f, .2f, .8f), "Inter-Regular", 18));
	Collider2DRectangle* rectColS = new Collider2DRectangle(80,80);
	Collider2DRectangle* rectColL = new Collider2DRectangle(160, 160);
	Collider2DCircle* circleColS = new Collider2DCircle(60);
	Collider2DCircle* circleColL = new Collider2DCircle(200);
	Collider2DLine* lineColS = new Collider2DLine(0,-25,0,25);
	Collider2DLine* lineColL = new Collider2DLine(0, -120, 0, 120);
	Collider2DPolygon* polyColS = new Collider2DPolygon({ glm::vec2(-75,0), glm::vec2(-40,-50), glm::vec2(40,-50), glm::vec2(75,0), glm::vec2(40, 50), glm::vec2(-40,50) });
	Collider2DPolygon* polyColL = new Collider2DPolygon({ glm::vec2(-150,0), glm::vec2(-80,-100), glm::vec2(80,-100), glm::vec2(150,0), glm::vec2(80, 100), glm::vec2(-80,100) });
	const int numCols = 8;
	Collider2D* cols[numCols] = { rectColS,rectColL,circleColS,circleColL, lineColS,lineColL, polyColS,polyColL };
	GameObject2D* g1 = addGameObject2D(new GameObject2D(glm::vec2(SCR_WIDTH/2-300, SCR_HEIGHT/2), 0, glm::vec2(1), glm::vec3(1), "", false, 0, rectColS));
	GameObject2D* g2 = addGameObject2D(new GameObject2D(glm::vec2(SCR_WIDTH/2, SCR_HEIGHT/2), 0, glm::vec2(1), glm::vec3(1), "", false, 0, rectColS));
	int g1ColInd = 0, g2ColInd = 0;

	while (!glfwWindowShouldClose(window)) {
		// update frame
		updateTime();
		resetSingleFrameInput();
		glfwPollEvents();

		updateObjects();

		// update colliders
		g1->translate((keyHeld("mvRight") - keyHeld("mvLeft")) * 200 * deltaTime, (keyHeld("mvDown") - keyHeld("mvUp")) * 200 * deltaTime);
		g1->rotate((keyHeld("rotCW") - keyHeld("rotCCW")) * 3 * deltaTime);
		if (keyPressed("swap1"))
			g1->collider = cols[(g1ColInd+=1) %= numCols];
		if (keyPressed("swap2"))
			g2->collider = cols[(g2ColInd+=1) %= numCols];

		textObjects[1]->text = "Colliding ? " + (g1->collidesWith(g2) ? std::string("true") : std::string("false")) + " | " + (g2->collidesWith(g1) ? std::string("true") : std::string("false"));

		// render
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		renderLines2D();
		render2D();
		glfwSwapBuffers(window);
		// set the close flag if the player presses the escape key
		if (keyPressed(GLFW_KEY_ESCAPE))
			glfwSetWindowShouldClose(window, true);
	}
	glfwTerminate();
}