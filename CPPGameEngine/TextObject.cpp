#pragma once
#include "stdafx.h"
#include "TextObject.h"

extern void renderText(std::string fontName, int fontSize, Shader& s, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color, bool centered = false);

void TextObject::draw(Shader s) {
	// TODO: a lot of these properties are hard-coded atm and should not be
	renderText("Inter-Regular", 24, s, text, 6, 6, 1.0f, glm::vec3(0.5, 0.8f, 0.2f));
}