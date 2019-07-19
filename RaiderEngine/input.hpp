#pragma once
#include "stdafx.h"
#include "graphics.hpp"

// mouse
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;
bool mousePressedLeft = false;  // whether or not the left mouse button was just pressed
bool mouseHeldLeft = false;  // whether or not the left mouse button is currently being held down
bool mouseReleasedLeft = false;  // whether or not the left mouse button was just released 
bool mousePressedRight = false;  // whether or not the right mouse button was just pressed
bool mouseHeldRight = false;  // whether or not the right mouse button is currently being held down
bool mouseReleasedRight = false;  // whether or not the right mouse button was just released 
bool f3Pressed = false;

/*
reset all input events that occur for a single frame only
*/
void resetSingleFrameInput() {
	// reset mouse events
	mousePressedLeft = false;
	mouseReleasedLeft = false;
	mousePressedRight = false;
	mouseReleasedRight = false;
	// reset pressed and released state for each key
	for (int i = 0; i < GLFW_KEY_LAST; ++i) {
		keyStates[i][pressed] = false;
		keyStates[i][released] = false;
	}
}

/*
toggle debugDraw on f3 press
*/
void updateDebugToggle(GLFWwindow* window) {
	// debug key update
	if (keyStates[GLFW_KEY_F3][pressed])
		debugDraw = !debugDraw;
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	player.camera.ProcessMouseMovement(xoffset, yoffset);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS) {
		keyStates[key][pressed] = true;
		keyStates[key][held] = true;
	}
	else if (action == GLFW_RELEASE) {
		keyStates[key][released] = true;
		keyStates[key][held] = false;
	}
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		if (action == GLFW_PRESS) {
			mousePressedLeft = true;
			mouseHeldLeft = true;
		}
		else if (action == GLFW_RELEASE) {
			mouseReleasedLeft = true;
			mouseHeldLeft = false;
		}
	}
	else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
		if (action == GLFW_PRESS) {
			mousePressedRight = true;
			mouseHeldRight = true;
		}
		else if (action == GLFW_RELEASE) {
			mouseReleasedRight = true;
			mouseHeldRight = false;
		}
	}
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	player.camera.ProcessMouseScroll(yoffset);
}