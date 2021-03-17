#pragma once
#include "stdafx.h"
#include "shader.hpp"

class TextObject {
public:
	inline static GLuint VAO, VBO;
	inline static unsigned int numGlpyhsBuffered = 0;
	std::string text;
	float x, y;
	glm::vec3 color;
	bool centered;
	std::string fontName;
	int fontSize;
	float scale;
	float depth = 0.1f;  // depth in NDC coordinates

	TextObject(std::string text, float x, float y, glm::vec3 color, std::string fontName, int fontSize, bool centered = false, float scale = 1.f) : text(text), x(x), y(y), color(color), fontName(fontName), fontSize(fontSize), centered(centered), scale(scale) {};

	static void initVertexObjects();

	virtual void update() {}

	/*
	render the text contained in this textObject
	@param s: the shader to use when rendering
	*/
	virtual void draw(Shader s);
};