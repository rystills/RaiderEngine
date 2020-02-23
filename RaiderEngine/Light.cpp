#include "stdafx.h"
#include "Light.hpp"

Light::Light(glm::vec3 position, float strength, glm::vec3 color) : position(position), color(color) {
	offColor = glm::vec3(color.r / 8, color.g / 8, color.b / 8);
	linear = 7 / strength * .7f;
	quadratic = 7 / strength * 1.8f;
	calculateMaxBrightness();
	calculateRadius();
}

void Light::setOn(bool nowOn) {
	if (nowOn != on) {
		on = nowOn;
		dirty = true;
	}
}

void Light::calculateMaxBrightness() {
	maxBrightness = std::fmaxf(std::fmaxf(color.r, color.g), color.b);
}

void Light::calculateRadius() {
	radius = (-linear + std::sqrt(linear * linear - 4 * quadratic * (1 - (256.0f / 5.0f) * maxBrightness))) / (2.0f * quadratic);
}