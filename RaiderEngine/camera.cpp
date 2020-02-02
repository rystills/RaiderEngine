#include "stdafx.h"
#include "camera.hpp"
#include "settings.hpp"
#include "timing.hpp"

Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(8), MouseSensitivity(0.1f), Zoom(45.f) {
	Position = position;
	WorldUp = up;
	setYaw(yaw);
	Pitch = pitch;
	updateCameraVectors();
}

Camera::Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(8), MouseSensitivity(0.1f), Zoom(45.f) {
	Position = glm::vec3(posX, posY, posZ);
	WorldUp = glm::vec3(upX, upY, upZ);
	setYaw(yaw);
	Pitch = pitch;
	updateCameraVectors();
}

void Camera::updateViewProj() {
	projection = glm::perspective(glm::radians(Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, near_plane, far_plane);
	view = glm::lookAt(Position, Position + Front, Up);;
}

void Camera::ProcessKeyboard(Camera_Movement direction) {
	if (!controllable) return;
	float velocity = MovementSpeed * deltaTime;
	if (direction == FORWARD)
		//Position -= glm::normalize(glm::cross(Right, WorldUp)) * velocity;
		Position += Front * velocity;
	if (direction == BACKWARD)
		//Position += glm::normalize(glm::cross(Right, WorldUp)) * velocity;
		Position -= Front * velocity;
	if (direction == LEFT)
		Position -= Right * velocity;
	if (direction == RIGHT)
		Position += Right * velocity;
}

void Camera::setYaw(float inYaw) {
	Yaw = fmod(inYaw, 360);
	while (Yaw < 0)
		Yaw += 360;
}

void Camera::moveFlycam() {
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		ProcessKeyboard(FORWARD);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		ProcessKeyboard(BACKWARD);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		ProcessKeyboard(LEFT);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		ProcessKeyboard(RIGHT);
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch) {
	if (!controllable) return;
	xoffset *= MouseSensitivity;
	yoffset *= MouseSensitivity;
	setYaw(Yaw + xoffset);
	Pitch += yoffset;

	// Make sure that when pitch is out of bounds, screen doesn't get flipped
	if (constrainPitch) {
		if (Pitch > 89.0f)
			Pitch = 89.0f;
		if (Pitch < -89.0f)
			Pitch = -89.0f;
	}

	// Update Front, Right and Up Vectors using the updated Euler angles
	updateCameraVectors();
}

void Camera::ProcessMouseScroll(float yoffset) {
	if (!controllable) return;
	if (Zoom >= 1.0f && Zoom <= 45.0f)
		Zoom -= yoffset;
	if (Zoom <= 1.0f)
		Zoom = 1.0f;
	if (Zoom >= 45.0f)
		Zoom = 45.0f;
}

void Camera::updateCameraVectors() {
	// Calculate the new Front vector
	glm::vec3 front;
	front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
	front.y = sin(glm::radians(Pitch));
	front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
	Front = glm::normalize(front);
	// Also re-calculate the Right and Up vector
	Right = glm::normalize(glm::cross(Front, WorldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
	Up = glm::normalize(glm::cross(Right, Front));
}