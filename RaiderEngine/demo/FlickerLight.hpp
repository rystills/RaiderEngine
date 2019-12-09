#pragma once
#include "../stdafx.h"
#include "../Light.hpp"

class FlickerLight : public Light {
public:
	float elapsedTime = 0, prevTime = 0, flickerWait = chooseFlickerWait(3);
	FlickerLight(glm::vec3 position, float strength, glm::vec3 color) :Light(position, strength, color) {
	}

	/*
	choose how long the light should wait before switching states again
	@param maxSeconds: the maximum time (in seconds) the light can wait
	@returns: the randomly chosen time (in seconds) the light should wait before switching states
	*/
	float chooseFlickerWait(float maxSeconds) {
		return (rand()) / static_cast <float> (RAND_MAX/maxSeconds);
	}

	void update(float deltaTime) override {
		elapsedTime += deltaTime;
		Light::update(deltaTime);
		// flicker once flickerWait time has elapsed
		if (prevTime + flickerWait < elapsedTime) {
			on = !on;
			prevTime += flickerWait;
			// choose a new time to wait within a small range when the light is off, and a much larger range when the light is on
			flickerWait = chooseFlickerWait(on ? 3 : .45f);
		}
	}
};