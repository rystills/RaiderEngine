#include "stdafx.h"
#if (COMPILE_DEMO == DEMO_BRICK_BREAKER)
#include "settings.hpp"
#include "input.hpp"
#include "GameManager.hpp"

int main() {
	initEngine();
	// directories
	setTextureDir("demos/demoBrickBreaker/images");
	setSoundDir("demos/demoBrickBreaker/sounds");
	setFontDir("demos/shared/fonts");

	// keybindings
	setKeyBinding("mvLeft", GLFW_KEY_A);
	setKeyBinding("mvRight", GLFW_KEY_D);
	setKeyBinding("restart", GLFW_KEY_ENTER);

	// fonts
	freetypeLoadFont("Inter-Regular", 18);
	freetypeLoadFont("Inter-Regular", 48);
	
	setVsync(false);
	setClearColor(.8f, .8f, 1, 1);
	addTextObject(new GameManager("Inter-Regular", 48));

	while (beginFrame(false)) {
		updateObjects();
		render(true);
		// set the close flag if the player presses the escape key
		if (keyPressed(GLFW_KEY_ESCAPE))
			glfwSetWindowShouldClose(window, true);
	}
	closeEngine();
}
#endif