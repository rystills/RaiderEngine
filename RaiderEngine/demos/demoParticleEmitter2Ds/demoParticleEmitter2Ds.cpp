#include "stdafx.h"
#include "GameObject2D.hpp"
#include "settings.hpp"
#include "timing.hpp"
#include "audio.hpp"
#include "FpsDisplay.hpp"
#include "input.hpp"
#include "ParticleEmitter2D.hpp"

int main() {
	// directories
	setTextureDir("demos/demoParticleEmitter2Ds/images");
	setSoundDir("demos/demoParticleEmitter2Ds/sounds");
	setFontDir("demos/shared/fonts");

	// initialization
	window = initGraphics();
	initAudio();
	freetypeLoadFont("Inter-Regular", 18);
	
	setVsync(false);
	clearColor = glm::vec4(.5f, .3f, .6f, 1);
	glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
	addTextObject(new FpsDisplay(6, 6, glm::vec3(1, 1, 1), "Inter-Regular", 18));
	TextObject* to = addTextObject(new TextObject("",6, 30, glm::vec3(1, 1, 1), "Inter-Regular", 18));
	ParticleEmitter2D* pe = addParticleEmitter2D(new ParticleEmitter2D(glm::vec2(SCR_WIDTH/2, SCR_HEIGHT/2), "star.png"));

	while (!glfwWindowShouldClose(window)) {
		// update frame
		updateTime();
		resetSingleFrameInput();
		glfwPollEvents();

		// update objects
		updateObjects();
		to->text = "num particles: " + std::to_string(pe->particles.size());

		// render
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		render2D();
		glfwSwapBuffers(window);
		// set the close flag if the player presses the escape key
		if (keyPressed(GLFW_KEY_ESCAPE))
			glfwSetWindowShouldClose(window, true);
	}
	glfwTerminate();
}