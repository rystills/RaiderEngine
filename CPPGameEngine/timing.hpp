#include "stdafx.h"
// this file is responsible for maintaining timing information within the main game loop
float deltaTime = 0.0f;
float lastFrame = 0.0f;

/*
update deltaTime based on the amount of time elapsed since the previous frame
*/
void updateTime() {
	float currentFrame = glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	// if fps goes below 30, slow down the game speed rather than trying to interpolate (this should prevent occasional jitters from breaking the physics)
	if (deltaTime > .034f) deltaTime = .034f;
	lastFrame = currentFrame;
}