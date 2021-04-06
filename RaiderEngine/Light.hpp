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
	bool dirty = true;
	bool enableShadows = true;
	Light(glm::vec3 position, bool enableShadows, float strength, glm::vec3 color);

	void setOn(bool nowOn);
	virtual void update() {}

protected:
	/*
	calculate the lights maximum brightness using its rgb components
	*/
	void calculateMaxBrightness();

	/*
	calcuate the light's radius given linear and quadratic constants and its max brightness
	*/
	void calculateRadius();

	void setPos(glm::vec3 const& newPos);
};