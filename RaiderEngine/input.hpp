#pragma once
#include "stdafx.h"

// mouse
inline float lastX = 0;
inline float lastY = 0;
inline bool firstMouse = true;
inline bool mousePressedLeft = false;  // whether or not the left mouse button was just pressed
inline bool mouseHeldLeft = false;  // whether or not the left mouse button is currently being held down
inline bool mouseReleasedLeft = false;  // whether or not the left mouse button was just released 
inline bool mousePressedRight = false;  // whether or not the right mouse button was just pressed
inline bool mouseHeldRight = false;  // whether or not the right mouse button is currently being held down
inline bool mouseReleasedRight = false;  // whether or not the right mouse button was just released 
inline bool f3Pressed = false;
enum keyState { pressed = 0, held = 1, released = 2 };
inline int keyStates[GLFW_KEY_LAST][3] = { 0 };
inline std::unordered_map<std::string, unsigned int> keyBindings;

void setKeyBinding(std::string action, unsigned int key);

bool keyHeld(int key);
bool keyHeld(std::string action);
bool keyPressed(int key);
bool keyPressed(std::string action);
bool keyReleased(int key);
bool keyReleased(std::string action);

/*
reset all input events that occur for a single frame only
*/
void resetSingleFrameInput();

/*
toggle debugDraw on f3 press
*/
void updateDebugToggle(GLFWwindow* window);

/*
callback for mouse movement
*/
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

/*
callback for keyboard keys
*/
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

/*
callback for mouse buttons
*/
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

/*
callback function for mouse scroll wheel
*/
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);