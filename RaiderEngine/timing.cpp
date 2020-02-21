#include "stdafx.h"
#include "terminalColors.hpp"
#include "timing.hpp"

void updateTime() {
	double currentFrame = glfwGetTime();
	// keep track of fps - updating the display once per second for readability
	++framesThisSecond;
	avgRenderTime = avgRenderTime * ((framesThisSecond - 1.0) / framesThisSecond) + frameRenderTime * (1.0 / framesThisSecond);
	avgUpdateTime = avgUpdateTime * ((framesThisSecond - 1.0) / framesThisSecond) + frameUpdateTime * (1.0 / framesThisSecond);
	if (currentFrame - lastTime >= 1) {
		// store lastTime in integer increments to keep fps honest across each increment
		lastTime = static_cast<int>(currentFrame);
		fps = framesThisSecond;
		framesThisSecond = 0;
		secondAvgRenderTime = avgRenderTime;
		secondAvgUpdateTime = avgUpdateTime;
	}
	double dtime = currentFrame - lastFrame;
	// if fps goes below 30, slow down the game speed rather than trying to interpolate (this should prevent occasional jitters from breaking the physics)
	if (dtime > .034) {
		if (totalTime != 0)
			WARNINGCOLOR(std::cout << "Framerate jitter detected last frame! Recorded a frame deltaTime above .034 (30fps) of " << dtime << std::endl)
			dtime = .034;
	}
	lastFrame = currentFrame;
	totalTime += dtime;
	// timing is measured using doubles for higher precision, but deltaTime is truncated to a float so that GameObjects can more easily work with it
	deltaTime = static_cast<float>(dtime);
}