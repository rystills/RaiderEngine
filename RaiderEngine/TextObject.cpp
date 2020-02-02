#pragma once
#include "stdafx.h"
#include "TextObject.hpp"

extern void renderText(std::string fontName, int fontSize, Shader& s, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color, bool centered, bool shouldUseShader);

void TextObject::draw(Shader s, bool shouldUseShader) {
	renderText(fontName, fontSize, s, text, x, y, 1.0f, color, centered, shouldUseShader);
}