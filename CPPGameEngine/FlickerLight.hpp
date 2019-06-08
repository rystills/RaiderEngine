#ifndef FLICKERLIGHT_H
#define FLICKERLIGHT_H

#include "Light.hpp"

class FlickerLight : public Light {
public:
	float elapsedTime = 0;
	FlickerLight(glm::vec3 position, glm::vec3 color, float strength) :Light(position, color, strength) { }
	
	void update(float deltaTime) override {
		elapsedTime += deltaTime;
		Light::update(deltaTime);
		on = (((int)elapsedTime) % 2);
	}
};

#endif