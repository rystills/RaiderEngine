#include "stdafx.h"
#include "terminalColors.hpp"
#include "timing.hpp"

void updateTime() {
	double currentFrame = glfwGetTime();
	// keep track of fps - updating the display once per second for readability
	++framesThisSecond;
	if (currentFrame - lastTime >= 1) {
		// store lastTime in integer increments to keep fps honest across each increment
		lastTime = static_cast<int>(currentFrame);
		fps = framesThisSecond;
		framesThisSecond = 0;
	}
	deltaTime = static_cast<float>(currentFrame - lastFrame);
	// if fps goes below 30, slow down the game speed rather than trying to interpolate (this should prevent occasional jitters from breaking the physics)
	if (deltaTime > .034) {
		if (totalTime != 0)
			WARNINGCOLOR(std::cout << "Framerate jitter detected last frame! Recorded a frame deltaTime above .034 (30fps) of " << deltaTime << std::endl)
		deltaTime = .034f;
	}
	lastFrame = currentFrame;
	totalTime += deltaTime;
}