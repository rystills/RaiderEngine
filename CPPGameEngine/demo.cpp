#include "stdafx.h"
// engine includes (import order matters here, at least for the time being)
#include "terminalColors.hpp"
#include "filesystem.hpp"
#include "timing.hpp"
#include "physics.hpp"
#include "settings.hpp"
#include "mapLoader.hpp"
#include "graphics.hpp"
#include "shader.hpp"

#include "model.hpp"
#include "GameObject.hpp"
#include "Light.hpp"

#include "rightClickObserve.hpp"
#include "mousePicking.hpp"

/*
draw a dot in the center of the screen, allowing the player to easily see which object is currently being moused over
*/
void drawCenterIndicator() {
	// convert center position into camera coordinates
	glm::mat4 M = glm::inverse(player.camera.projection*player.camera.view);
	glm::vec4 lRayStart_world = M * glm::vec4(0, 0, 0, 1); lRayStart_world /= lRayStart_world.w;
	debugDrawPoint(glm::vec3(lRayStart_world.x, lRayStart_world.y, lRayStart_world.z), glm::vec3(255, 255, 255));
}

int main() {
	// note: uncomment me and set me to the proper directory if you need to run Dr. Memory
	// _chdir("C:\\Users\\Ryan\\Documents\\git-projects\\CPPGameEngine\\CPPGameEngine");
	GLFWwindow* window = initWindow();
	initGBuffer();
	initDepthMaps();
	initPhysics();
	initFreetype();
	initBuffers();
	player.init();
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
	glEnable(GL_CULL_FACE);
	glEnable(GL_PROGRAM_POINT_SIZE);

	loadShaders();
	Model::loadDefaultMaterialMaps();

	// load map
	loadMap("hallway");
	// enable anisotropic filtering if supported
	applyAnisotropicFiltering();

	while (!glfwWindowShouldClose(window)) {
		updateDebugToggle(window);

		// update frame
		updateTime();
		resetSingleFrameInput();
		glfwPollEvents();

		// update physics
		NewtonUpdate(world, deltaTime);

		// update player
		player.update(window, deltaTime);
		// update objects
		for (int i = 0; i < gameObjects.size(); ++i)
			gameObjects[i]->update(deltaTime);
		for (int i = 0; i < lights.size(); ++i)
			lights[i]->update(deltaTime);

		// picking and object info display
		if (displayString.size() == 0) {
			UpdatePickBody(deltaTime);
			if (!m_targetPicked)
				checkDisplayObject();
		}
		else
			updateDisplayString();

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
	}
	cleanupPhysics();
	glfwTerminate();
	// delete object and model data
	gameObjects.clear();
	models.clear();
}