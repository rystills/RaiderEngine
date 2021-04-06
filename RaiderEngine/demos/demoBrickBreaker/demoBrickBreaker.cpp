#include "stdafx.h"
#if (COMPILE_DEMO == DEMO_BRICK_BREAKER)
#include "settings.hpp"
#include "input.hpp"
#include "GameManager.hpp"

int main() {
	// directories
	setTextureDir("demos/demoBrickBreaker/images");
	setSoundDir("demos/demoBrickBreaker/sounds");
	setFontDir("demos/shared/fonts");

	// keybindings
	setKeyBinding("mvLeft", GLFW_KEY_A);
	setKeyBinding("mvRight", GLFW_KEY_D);
	setKeyBinding("restart", GLFW_KEY_ENTER);

	initEngine();

	// fonts
	freetypeLoadFont("Inter-Regular", 18);
	freetypeLoadFont("Inter-Regular", 48);

	setVsync(false);
	setClearColor(.8f, .8f, 1, 1);
	addTextObject(new GameManager("Inter-Regular", 48));

	while (beginFrame(false)) {
		checkDemoToggles();
		updateObjects();
		render(true);
	}
	closeEngine();
}
#endif