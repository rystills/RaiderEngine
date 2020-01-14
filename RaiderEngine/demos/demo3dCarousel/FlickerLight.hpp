#pragma once
#include "stdafx.h"
#include "Light.hpp"

class FlickerLight : public Light {
public:
	float elapsedTime = 0, prevTime = 0, flickerWait = chooseFlickerWait(3);
	FlickerLight(glm::vec3 position, float strength, glm::vec3 color) : Light(position, strength, color) { }

	/*
	choose how long the light should wait before switching states again
	@param maxSeconds: the maximum time (in seconds) the light can wait
	@returns: the randomly chosen time (in seconds) the light should wait before switching states
	*/
	float chooseFlickerWait(float maxSeconds);

	void update() override;
};