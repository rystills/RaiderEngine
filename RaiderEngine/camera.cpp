#include "stdafx.h"
#include "camera.hpp"
#include "settings.hpp"
#include "timing.hpp"
#include "input.hpp"

Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), movementSpeed(4), MouseSensitivity(0.1f), Zoom(45.f) {
	Position = position;
	WorldUp = up;
	setYaw(yaw);
	Pitch = pitch;
	updateCameraVectors();
}

Camera::Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), movementSpeed(4), MouseSensitivity(0.1f), Zoom(45.f) {
	Position = glm::vec3(posX, posY, posZ);
	WorldUp = glm::vec3(upX, upY, upZ);
	setYaw(yaw);
	Pitch = pitch;
	updateCameraVectors();
}

void Camera::updateViewProj() {
	projection = glm::perspective(glm::radians(Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, near_plane, far_plane);
	view = glm::lookAt(Position, Position + Front, Up);
}

void Camera::setYaw(float inYaw) {
	Yaw = fmodf(inYaw, 360);
	if (Yaw < 0)
		Yaw += 360;
}

void Camera::moveFlycam() {
	if (!controllable) return;
	float velocity = (keyHeld("run") ? sprintSpeed : movementSpeed) * deltaTime;
	Position += (float)(keyHeld("mvForward") - keyHeld("mvBackward")) * Front * velocity;
	Position += (float)(keyHeld("mvRight") - keyHeld("mvLeft")) * Right * velocity;
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch) {
	if (!controllable) return;
	setYaw(Yaw + xoffset*MouseSensitivity);
	Pitch += yoffset*MouseSensitivity;

	// Make sure that when pitch is out of bounds, screen doesn't get flipped
	if (constrainPitch)
		Pitch = glm::min(glm::max(Pitch,-89.f),89.f);

	// Update Front, Right and Up Vectors using the updated Euler angles
	updateCameraVectors();
}

void Camera::ProcessMouseScroll(float yoffset) {
	if (!controllable) return;
	Zoom = glm::min(glm::max(Zoom - yoffset*4, 1.f), 45.f);
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