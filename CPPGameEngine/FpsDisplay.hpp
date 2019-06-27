#pragma once
#include "TextObject.hpp"
#include "timing.hpp"

class FpsDisplay : public TextObject {
public:
	FpsDisplay() : TextObject("") {}

	void update(float deltaTime) override { 
		text = "fps: " + std::to_string(fps);
	}
};