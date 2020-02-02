#pragma once
#include "stdafx.h"
#include "shader.hpp"

class TextObject {
public:
	std::string text;
	float x, y;
	glm::vec3 color;
	bool centered;
	std::string fontName;
	int fontSize;
	TextObject(std::string text, float x, float y, glm::vec3 color, std::string fontName, int fontSize, bool centered = false) : text(text), x(x), y(y), color(color), fontName(fontName), fontSize(fontSize), centered(centered) {};

	virtual void update() {}

	/*
	render the text contained in this textObject
	@param s: the shader to use when rendering
	*/
	virtual void draw(Shader s, bool shouldUseShader);
};