#include "stdafx.h"
#include "timing.hpp"

void updateTime() {
	float currentFrame = glfwGetTime();
	++framesThisSecond;
	if (currentFrame - lastTime > 1) {
		lastTime = (int)currentFrame;
		fps = framesThisSecond;
		framesThisSecond = 0;
	}
	deltaTime = currentFrame - lastFrame;
	// if fps goes below 30, slow down the game speed rather than trying to interpolate (this should prevent occasional jitters from breaking the physics)
	if (deltaTime > .034f) deltaTime = .034f;
	lastFrame = currentFrame;
	totalTime += deltaTime;
}