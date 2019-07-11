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

int main() {
	// note: uncomment me and set me to the proper directory if you need to run Dr. Memory
	// _chdir("C:\\Users\\Ryan\\Documents\\git-projects\\CPPGameEngine\\CPPGameEngine");
	window = initGraphics();
	clearColor = glm::vec4(.6f, .3f, .5f, 1);
	initAudio();
	player.init();

	// load map
	setMapDir("demo/maps");
	setModelDir("demo/models");
	setTextureDir("demo/textures");
	setSoundDir("demo/sounds");
	setFontDir("demo/fonts");
	loadMap("hallway");
	// enable anisotropic filtering if supported
	applyAnisotropicFiltering();
	// add fps indicator
	freetypeLoadFont("Inter-Regular", 18);
	freetypeLoadFont("Inter-Regular", 24);
	textObjects.emplace_back(new FpsDisplay(6,6,glm::vec3(1,1,1),18));
	textObjects.emplace_back(new TextObject("Press f3 to toggle physics wireframes",6,30, glm::vec3(.8f, .2f, .5f), 24));
	textObjects.emplace_back(new TextObject("Use WASD to move, space to jump, and left shift to sprint", 6, 60, glm::vec3(.5f, .8f, .2f), 24));
	textObjects.emplace_back(new TextObject("Press left mouse to grab objects, and right mouse to observe", 6, 90, glm::vec3(.2f, .5f, .8f), 24));
	playSound("Alien_Spaceship_Atmosphere.ogg");

	while (!glfwWindowShouldClose(window)) {
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