#pragma once
#include "stdafx.h"
#include "TextObject.hpp"


class FpsDisplay : public TextObject {
public:
	FpsDisplay(float x, float y, glm::vec3 color, std::string fontName, int fontSize, bool centered = false) : TextObject("",x,y,color,fontName, fontSize,centered) {}

	void update() override;
};