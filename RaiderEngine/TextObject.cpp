#pragma once
#include "stdafx.h"
#include "TextObject.hpp"

extern void renderText(std::string fontName, int fontSize, Shader& s, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color, bool centered);


void TextObject::initVertexObjects() {
	// Configure VAO and VBO
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void TextObject::draw(Shader s) {
	renderText(fontName, fontSize, s, text, x, y, 1.0f, color, centered);
}