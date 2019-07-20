#include "stdafx.h"
// engine includes (import order matters here, at least for the time being)
#include "../terminalColors.hpp"
#include "../filesystem.hpp"
#include "../timing.hpp"
#include "../physics.hpp"
#include "../settings.hpp"
#include "../mapLoader.hpp"
#include "../graphics.hpp"
#include "../shader.hpp"

#include "../model.hpp"
#include "../GameObject.hpp"
#include "../Light.hpp"

#include "../FpsDisplay.hpp"
#include "../audio.hpp"

#include "mouseInteraction.hpp"
/*
draw a dot in the center of the screen, allowing the player to easily see which object is currently being moused over
*/
void drawCenterIndicator() {
	// convert center position into camera coordinates
	glm::mat4 M = glm::inverse(player.camera.projection*player.camera.view);
	glm::vec4 lRayStart_world = M * glm::vec4(0, 0, 0, 1); lRayStart_world /= lRayStart_world.w;
	queueDrawPoint(glm::vec3(lRayStart_world.x, lRayStart_world.y, lRayStart_world.z), glm::vec3(255, 255, 255));
	drawPoints();
}

int scene = 0;
/*
attempt to switch to the selected map
@param mapNum: the number corresponding to the map we should switch to
*/
void checkSwitchMap(int mapNum) {
	// base case: can't switch to the same scene or an invalid scene
	if (mapNum == scene || (mapNum < 1 || mapNum > 3))
		return;
	// clear the current scene
	for (auto&& kv : gameObjects)
		for (int i = 0; i < kv.second.size(); ++i)
			if (kv.second[i]->usePhysics)
				gScene->removeActor(*kv.second[i]->body);
	gameObjects.clear();
	models.clear();
	lights.clear();
	// load the new scene
	scene = mapNum;
	switch (scene) {
	case 1:
		clearColor = glm::vec4(.6f, .3f, .5f, 1);
		ambientStrength = 0;
		loadMap("hallway");
		break;
	case 2:
		clearColor = glm::vec4(0, .75f, 1, 1);
		ambientStrength = 0.4f;
		loadMap("field");
		break;
	case 3:
		clearColor = glm::vec4(0, 0, .2f, 1);
		ambientStrength = 0.15f;
		loadMap("bookshelf");
		break;
	}
}
int main() {
	// initialization
	window = initGraphics();
	initAudio();
	player.init();

	// directories
	setMapDir("demo/maps");
	setModelDir("demo/models");
	setTextureDir("demo/textures");
	setSoundDir("demo/sounds");
	setFontDir("demo/fonts");

	// sound
	playSound("Julie_Li_-_01_-_resound.ogg");

	// demo settings
	checkSwitchMap(1);
	
	// enable anisotropic filtering if supported
	applyAnisotropicFiltering();
	// add fps indicator
	freetypeLoadFont("Inter-Regular", 18);
	freetypeLoadFont("Inter-Regular", 24);
	textObjects.emplace_back(new FpsDisplay(6,6,glm::vec3(1,1,1),18));
	textObjects.emplace_back(new TextObject("Press f3 to toggle physics wireframes",6,30, glm::vec3(.8f, .2f, .5f), 24));
	textObjects.emplace_back(new TextObject("Use WASD to move, space to jump, and left shift to sprint", 6, 60, glm::vec3(.5f, .8f, .2f), 24));
	textObjects.emplace_back(new TextObject("Press left mouse to grab objects, and right mouse to observe", 6, 90, glm::vec3(.2f, .5f, .8f), 24));
	textObjects.emplace_back(new TextObject("Press 1-3 to switch between the demo scenes", 6, 120, glm::vec3(1, 1, 0), 24));

	while (!glfwWindowShouldClose(window)) {
		// switch scenes on number key press
		if (keyStates[GLFW_KEY_1][pressed])
			checkSwitchMap(1);
		else if (keyStates[GLFW_KEY_2][pressed])
			checkSwitchMap(2);
		else if (keyStates[GLFW_KEY_3][pressed])
			checkSwitchMap(3);

		// create an extremely simple "day/night cycle" in scene 2 by mapping the ambient lighting strength to a sin wave 
		if (scene == 2) 
			ambientStrength = .5f*sin(totalTime) + .5f;
		// update frame
		updateTime();
		resetSingleFrameInput();
		glfwPollEvents();

		// update physics
		updatePhysics(deltaTime);

		// update player
		player.update(deltaTime);
		// update objects
		updateObjects();

		// picking and object info display
		if (displayString.size() > 0)
			// we're currently observing something; nothing to do until the user right clicks again
			updateDisplayString();
		else {
			if (!gMouseJoint)
				// there's nothing held at the moment, so check if the user is trying to observe something
				checkDisplayObject();
			if (displayString.size() == 0)
				// the user didn't try to observe something, so check if the user is holding or trying to grab something
				updateHeldBody(deltaTime);
		}
		
		// render
		renderDepthMap();
		renderGeometryPass();
		renderLightingPass();
		debugDrawLightCubes();
		renderLines();
		if (displayString.size() == 0)
			drawCenterIndicator();
		renderText();
		if (displayString.size() != 0)
			renderText("Inter-Regular", 24, *shaders["textShader"], displayString, SCR_WIDTH / 2, SCR_HEIGHT / 2, 1.0f, glm::vec3(1, 1, 1), true);
		glfwSwapBuffers(window);
		// set the close flag if the player presses the escape key
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);
	}
	glfwTerminate();
	// cleanup all of our data manually (unnecessary here, but good practice nonetheless)
	cleanupPhysics();
	gameObjects.clear();
	models.clear();
	sounds.clear();
}