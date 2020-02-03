#include "stdafx.h"
#if (COMPILE_DEMO == DEMO_3D_CAROUSEL)
#include "Light.hpp"
#include "timing.hpp"
#include "FlickerLight.hpp"

float FlickerLight::chooseFlickerWait(float maxSeconds) {
	return (rand()) / static_cast <float> (RAND_MAX / maxSeconds);
}

void FlickerLight::update() {
	elapsedTime += deltaTime;
	Light::update();
	// flicker once flickerWait time has elapsed
	if (prevTime + flickerWait < elapsedTime) {
		on = !on;
		prevTime += flickerWait;
		// choose a new time to wait within a small range when the light is off, and a much larger range when the light is on
		flickerWait = chooseFlickerWait(on ? 3 : .45f);
	}
}
#endif