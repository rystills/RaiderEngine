#pragma once
#include "TextObject.hpp"
#include "timing.hpp"

class FpsDisplay : public TextObject {
public:
	FpsDisplay(float x, float y, glm::vec3 color, int fontSize, bool centered = false) : TextObject("",x,y,color,fontSize,centered) {}

	void update(float deltaTime) override { 
		text = "fps: " + std::to_string(fps);
	}
};