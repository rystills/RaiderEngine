#pragma once
#include "stdafx.h"

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera {
public:
	// Camera Attributes
	glm::vec3 Position;
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;
	// Euler Angles
	float Yaw;
	float Pitch;
	// Camera options
	float MovementSpeed;
	float MouseSensitivity;
	float Zoom;
	bool controllable = true;

	// matrices and clipping planes
	glm::mat4 projection, view;
	float near_plane = 0.1f, far_plane = 1000.0f;

	// Constructor with vectors
	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = 270, float pitch = 0);
	
	// Constructor with scalar values
	Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch);

	/*
	update view and projection matrices, to be used for raycasting, rendering, etc..
	*/
	void updateViewProj();

	// Processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
	void ProcessKeyboard(Camera_Movement direction);

	/*
	set the camera's yaw value, clamping it to the range of 0-359
	@param inYaw: the new yaw value to use
	*/
	void setYaw(float inYaw);

	/*
	process keyboard events for WASD, enabling control of a simple flycam
	*/
	void moveFlycam();

	// Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
	void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);

	// Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
	void ProcessMouseScroll(float yoffset);

	// Calculates the front vector from the Camera's (updated) Euler Angles
	void updateCameraVectors();
};