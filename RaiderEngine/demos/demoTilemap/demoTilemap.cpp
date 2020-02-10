#include "stdafx.h"
#if (COMPILE_DEMO == DEMO_TILEMAP)
#include "GameObject2D.hpp"
#include "settings.hpp"
#include "timing.hpp"
#include "audio.hpp"
#include "FpsDisplay.hpp"
#include "input.hpp"
#include "Tilemap.hpp"

int main() {
	initEngine();
	// directories
	setFontDir("demos/shared/fonts");
	setTextureDir("demos/demoTilemap/images");

	// fonts
	freetypeLoadFont("Inter-Regular", 18);

	addTilemap(new Tilemap("tilemap.png",64,glm::vec2(16,10),glm::vec2(0)));

	setVsync(false);
	setClearColor(.85f, .85f, 1.f, 1.f);
	addTextObject(new FpsDisplay(6, SCR_HEIGHT - 20, glm::vec3(1.f), "Inter-Regular", 18));

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