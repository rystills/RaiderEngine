#include "stdafx.h"
#if (COMPILE_DEMO == DEMO_3D_CAROUSEL)
#include "settings.hpp"
#include "mapLoader.hpp"
#include "mouseInteraction.hpp"
#include "audio.hpp"
#include "ObjectRegistry.hpp"
#include "FpsDisplay.hpp"
#include "Compass.hpp"
#include "PlayerBase.hpp"
#include "PlayerSpawn.hpp"
#include "timing.hpp"

/*
queue a dot in the center of the screen to be rendered, allowing the player to easily see which object is currently being moused over
*/
void queueCenterIndicator() {
	// convert center position into camera coordinates
	glm::mat4 M = glm::inverse(mainCam->projection*mainCam->view);
	glm::vec4 lRayStart_world = M * glm::vec4(0, 0, 0, 1); lRayStart_world /= lRayStart_world.w;
	queueDrawPoint(glm::vec3(lRayStart_world.x, lRayStart_world.y, lRayStart_world.z), glm::vec3(255, 255, 255));
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
		for (unsigned int i = 0; i < kv.second.size(); ++i)
			if (kv.second[i]->usePhysics)
				gScene->removeActor(*kv.second[i]->body);
	gameObjects.clear();
	models.clear();
	lights.clear();
	// load the new scene
	scene = mapNum;
	switch (scene) {
	case 1:
		setClearColor(.6f, .3f, .5f, 1);
		ambientStrength = 0;
		loadMap("hallway");
		break;
	case 2:
		setClearColor(0, .75f, 1, 1);
		ambientStrength = 0.4f;
		loadMap("field");
		break;
	case 3:
		setClearColor(0, 0, .2f, 1);
		ambientStrength = 0.15f;
		loadMap("bookshelf");
		break;
	}
}

int main() {
	// initialization
	initEngine();
	PlayerBase player;
	player.init();
	PlayerSpawn::player = &player;
	objectRegistry = new ObjectRegistry();

	// directories
	setMapDir("demos/demo3dCarousel/maps");
	setModelDir("demos/demo3dCarousel/models");
	setTextureDir("demos/demo3dCarousel/textures");
	setSoundDir("demos/demo3dCarousel/sounds");
	setFontDir("demos/shared/fonts");

	setKeyBinding("loadScene1", GLFW_KEY_1);
	setKeyBinding("loadScene2", GLFW_KEY_2);
	setKeyBinding("loadScene3", GLFW_KEY_3);
	setKeyBinding("run", GLFW_KEY_LEFT_SHIFT);
	setKeyBinding("jump", GLFW_KEY_SPACE);
	setKeyBinding("crouch", GLFW_KEY_LEFT_CONTROL);
	setKeyBinding("mvLeft", GLFW_KEY_A);
	setKeyBinding("mvRight", GLFW_KEY_D);
	setKeyBinding("mvForward", GLFW_KEY_W);
	setKeyBinding("mvBackward", GLFW_KEY_S);

	// sound
	playSound("Julie_Li_-_01_-_resound.ogg");

	// demo settings
	checkSwitchMap(1);
	
	// enable anisotropic filtering if supported
	applyAnisotropicFiltering();
	// add fps indicator
	freetypeLoadFont("Inter-Regular", 18);
	freetypeLoadFont("Inter-Regular", 24);
	addTextObject(new FpsDisplay(6,6,glm::vec3(1,1,1), "Inter-Regular", 18));
	addTextObject(new TextObject("Press f3 to toggle physics wireframes",6,30, glm::vec3(.8f, .2f, .5f), "Inter-Regular", 24));
	addTextObject(new TextObject("Use WASD to move, space to jump, left ctrl to crouch, and left shift to sprint", 6, 60, glm::vec3(.5f, .8f, .2f), "Inter-Regular", 24));
	addTextObject(new TextObject("Press left mouse to grab objects, and right mouse to observe hovered objects", 6, 90, glm::vec3(.2f, .5f, .8f), "Inter-Regular", 24));
	addTextObject(new TextObject("Press 1-3 to switch between the demo scenes", 6, 120, glm::vec3(1, 1, 0), "Inter-Regular", 24));
	TextObject* displayStringIndicator = addTextObject(new TextObject("", SCR_WIDTH / 2.f, SCR_HEIGHT / 2.f,glm::vec3(1.f),"Inter-Regular",24,true));
	addGameObject2D(new Compass(glm::vec2(1146,586),0,glm::vec2(1), glm::vec3(1), "UI/compass.png"));

	while (beginFrame()) {
		updateDebugToggle(window);
		// switch scenes on number key press
		if (keyPressed("loadScene1"))
			checkSwitchMap(1);
		else if (keyPressed("loadScene2"))
			checkSwitchMap(2);
		else if (keyPressed("loadScene3"))
			checkSwitchMap(3);

		// create an extremely simple "day/night cycle" in scene 2 by mapping the ambient lighting strength to a sin wave 
		if (scene == 2) 
			ambientStrength = .5f*static_cast<float>(sin(totalTime)) + .5f;

		// update player
		player.update();
		// update objects
		updateObjects();

		// picking and object info display
		if (displayString.size() > 0) {
			// we're currently observing something; nothing to do until the user right clicks again
			if (updateDisplayString())
				displayStringIndicator->text = "";
		}
		else {
			// check if there's nothing held at the moment and the user is trying to observe something
			if (!gMouseJoint && checkDisplayObject())
				displayStringIndicator->text = displayString;
			if (displayString.size() == 0)
				// the user didn't try to observe something, so check if the user is holding or trying to grab something
				updateHeldBody();
		}
		
		// render
		if (displayString.size() == 0)
			queueCenterIndicator();
		render();
		// set the close flag if the player presses the escape key
		if (keyPressed(GLFW_KEY_ESCAPE))
			glfwSetWindowShouldClose(window, true);
	}
	closeEngine();
}
#endif