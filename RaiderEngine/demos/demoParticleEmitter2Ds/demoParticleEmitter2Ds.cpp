#include "stdafx.h"
#if (COMPILE_DEMO == DEMO_PARTICLE_EMITTER_2DS)
#include "GameObject2D.hpp"
#include "settings.hpp"
#include "timing.hpp"
#include "audio.hpp"
#include "FpsDisplay.hpp"
#include "input.hpp"
#include "ParticleEmitter2D.hpp"
#include "SparkleEmitter.hpp"
#include "FireEmitter.hpp"
#include "RainEmitter.hpp"
#include "CloudEmitter.hpp"

int main() {
	initEngine();
	// directories
	setTextureDir("demos/demoParticleEmitter2Ds/images");
	setSoundDir("demos/demoParticleEmitter2Ds/sounds");
	setFontDir("demos/shared/fonts");

	// fonts
	freetypeLoadFont("Inter-Regular", 18);
	
	setVsync(false);
	setClearColor(0, 0, 0, 1);
	addGameObject2D(new GameObject2D(glm::vec2(0), 0, glm::vec2(1), glm::vec3(1), "background.png"));
	addTextObject(new FpsDisplay(6, UI_TARGET_HEIGHT - 20.f, glm::vec3(1, 1, 1), "Inter-Regular", 18));
	TextObject* to1 = addTextObject(new TextObject("",6, UI_TARGET_HEIGHT - 44.f, glm::vec3(1, 1, 1), "Inter-Regular", 18));
	TextObject* to2 = addTextObject(new TextObject("", 6, UI_TARGET_HEIGHT - 68.f, glm::vec3(1, 1, 1), "Inter-Regular", 18));
	ParticleEmitter2D* pes = addParticleEmitter2D(new SparkleEmitter(glm::vec2(TARGET_WIDTH/2, TARGET_HEIGHT/2)));
	ParticleEmitter2D* pef = addParticleEmitter2D(new FireEmitter(glm::vec2(TARGET_WIDTH - 300, TARGET_HEIGHT - 100)));
	ParticleEmitter2D* per = addParticleEmitter2D(new RainEmitter(glm::vec2(230, 130)));
	ParticleEmitter2D* pecs[5];
	for (int i = 0; i < 5; ++i)
		pecs[i] = addParticleEmitter2D(new CloudEmitter(glm::vec2(150 + 40*i, 100)));

	while (beginFrame(false)) {
		checkDemoToggles();
		updateObjects();
		int numPecParts = 0, numPecRecycled = 0;
		for (int i = 0; i < 5; numPecParts += pecs[i]->particles.size(), numPecRecycled += pecs[i++]->recycledParticleInds.size());
		to1->text = "total particle count: " + std::to_string(pes->particles.size() + pef->particles.size() + numPecParts + per->particles.size());
		to2->text = "num recycled particles: " + std::to_string(pes->recycledParticleInds.size() + pef->recycledParticleInds.size() + numPecRecycled + per->recycledParticleInds.size());
		render(true);
		// set the close flag if the player presses the escape key
		if (keyPressed(GLFW_KEY_ESCAPE))
			glfwSetWindowShouldClose(window, true);
	}
	closeEngine();
}
#endif