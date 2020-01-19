#pragma once
#include "stdafx.h"

class Light {
public:
	glm::vec3 position, color, offColor;
	float linear, quadratic;
	int constant = 1;
	float radius;
	float maxBrightness;
	bool on = true;
	Light(glm::vec3 position, float strength, glm::vec3 color);
	
	virtual void update() {}

private:
	/*
	calculate the lights maximum brightness using its rgb components
	*/
	void calculateMaxBrightness();

	/*
	calcuate the light's radius given linear and quadratic constants and its max brightness
	*/
	void calculateRadius();
};