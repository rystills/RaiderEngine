#pragma once
#include "TextObject.hpp"


class FpsDisplay : public TextObject {
public:
	FpsDisplay(float x, float y, glm::vec3 color, int fontSize, bool centered = false) : TextObject("",x,y,color,fontSize,centered) {}

	void update() override;
};