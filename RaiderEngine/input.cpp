#include "stdafx.h"
#include "input.hpp"
#include "settings.hpp"

void setKeyBinding(std::string action, unsigned int key) {
	keyBindings[action] = key;
}

bool keyHeld(int key) {
	return keyStates[key][held];
}
bool keyHeld(std::string action) {
	return keyStates[keyBindings[action]][held];
}
bool keyPressed(int key) {
	return keyStates[key][pressed];
}
bool keyPressed(std::string action) {
	return keyStates[keyBindings[action]][pressed];
}
bool keyReleased(int key) {
	return keyStates[key][released];
}
bool keyReleased(std::string action) {
	return keyStates[keyBindings[action]][released];
}

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

void checkDemoToggles() {
	// fullscreen toggle
	if (keyPressed(GLFW_KEY_F11))
		setWindowMode(fullScreen ? 1280 : MONITOR_WIDTH, fullScreen ? 720 : MONITOR_HEIGHT, !fullScreen);

	// debug toggle
	if (keyStates[GLFW_KEY_F3][pressed])
		debugDraw = !debugDraw;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	if (firstMouse)
		firstMouse = false;
	else {
		float xoffset = static_cast<float>(xpos) - lastX;
		float yoffset = lastY - static_cast<float>(ypos); // reversed since y-coordinates go from bottom to top
		mainCam->ProcessMouseMovement(xoffset, yoffset);
	}
	lastX = static_cast<float>(xpos);
	lastY = static_cast<float>(ypos);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS) {
		keyStates[key][pressed] = true;
		keyStates[key][held] = true;
	}
	else if (action == GLFW_RELEASE) {
		keyStates[key][released] = true;
		keyStates[key][held] = false;
	}
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
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

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	mainCam->ProcessMouseScroll(static_cast<float>(yoffset));
}