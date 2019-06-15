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

/*
reset all input events that occur for a single frame only
*/
void resetSingleFrameInput() {
	// reset mouse events
	mousePressedLeft = false;
	mouseReleasedLeft = false;
	mousePressedRight = false;
	mouseReleasedRight = false;
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

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		if (action == GLFW_PRESS) {
			mousePressedLeft = true;
			mouseHeldLeft = true;
		}
		else {
			mouseReleasedLeft = true;
			mouseHeldLeft = false;
		}
	}
	else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
		if (action == GLFW_PRESS) {
			mousePressedRight = true;
			mouseHeldRight = true;
		}
		else {
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