#pragma once
#include "stdafx.h"
#include "shader.hpp"

class TextObject {
public:
	std::string text;
	TextObject(std::string text) : text(text) {}

	virtual void update(float deltaTime) {}

	virtual void draw(Shader s);
};