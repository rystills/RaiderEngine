#pragma once
#include "stdafx.h"
// this file is responsible for maintaining timing information within the main game loop
float totalTime = 0.0f;
float deltaTime = 0.0f;
float lastFrame = 0.0f;
int framesThisSecond = 0;
int fps = 0;
float lastTime = 0;

/*
update deltaTime based on the amount of time elapsed since the previous frame
*/
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