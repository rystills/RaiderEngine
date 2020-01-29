#include "stdafx.h"
#include "GameObject2D.hpp"
#include "settings.hpp"
#include "timing.hpp"
#include "audio.hpp"
#include "FpsDisplay.hpp"
#include "input.hpp"
#include "ParticleEmitter2D.hpp"
#include "SparkleEmitter.hpp"
#include "FireEmitter.hpp"

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
	clearColor = glm::vec4(0,0,0,1);
	glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
	addTextObject(new FpsDisplay(6, 6, glm::vec3(1, 1, 1), "Inter-Regular", 18));
	TextObject* to = addTextObject(new TextObject("",6, 30, glm::vec3(1, 1, 1), "Inter-Regular", 18));
	ParticleEmitter2D* pes = addParticleEmitter2D(new SparkleEmitter(glm::vec2(SCR_WIDTH/2, SCR_HEIGHT/2)));
	ParticleEmitter2D* pef = addParticleEmitter2D(new FireEmitter(glm::vec2(SCR_WIDTH - 300, SCR_HEIGHT - 100)));

	while (!glfwWindowShouldClose(window)) {
		// update frame
		updateTime();
		resetSingleFrameInput();
		glfwPollEvents();

		// update objects
		updateObjects();
		to->text = "num particles: " + std::to_string(pes->particles.size() + pef->particles.size());

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