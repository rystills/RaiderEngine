#include "stdafx.h"
#include "GameObject2D.hpp"
#include "model.hpp"
#include "settings.hpp"

GameObject2D::GameObject2D(glm::vec2 position, float rotation, glm::vec2 scale, glm::vec3 color, std::string spriteName, bool posIsCenter, float depth, Collider2D* collider) : 
	position(position), rotation(rotation), color(color), scaleVal(scale), depth(depth), collider(collider) {
	sprite = (spriteName == "" ? Model::defaultDiffuseMap : Model::loadTextureSimple(spriteName));
	recalculateHalfExtents();
	if (posIsCenter) {
		center = position;
		position -= halfExtents;
	}
	else
		center = position + halfExtents;
}

void GameObject2D::initStaticVertexBuffer() {
	// Configure VAO and temporary VBO
	GLuint VBO;
	GLfloat vertices[] = {
		// Pos      // Tex
		0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
	};

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindVertexArray(VAO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
	
	// configure VBO for transform (model) instanced rendering (attribs 1-4 contain the model matrix in the 2D shader)
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glGenBuffers(1, &instancedModelVBO);
	glBindBuffer(GL_ARRAY_BUFFER, instancedModelVBO);
	GLsizei vec4Size = sizeof(glm::vec4);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (GLvoid*)0);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (GLvoid*)(vec4Size));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (GLvoid*)(2 * vec4Size));
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (GLvoid*)(3 * vec4Size));
	
	glVertexAttribDivisor(1, 1);
	glVertexAttribDivisor(2, 1);
	glVertexAttribDivisor(3, 1);
	glVertexAttribDivisor(4, 1);

	// configure VBO for color instanced rendering
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glGenBuffers(1, &instancedColorVBO);
	glBindBuffer(GL_ARRAY_BUFFER, instancedColorVBO);
	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glVertexAttribDivisor(5, 1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void GameObject2D::recalculateHalfExtents() {
	halfExtents = { scaleVal.x * sprite.width * .5f, scaleVal.y * sprite.height * .5f };
}

void GameObject2D::setCenter(glm::vec2 newCenter) {
	center = newCenter;
	position = center - halfExtents;
	isDirty = true;
}
void GameObject2D::setCenter(float x, float y) {
	center.x = x;
	center.y = y;
	position = center - halfExtents;
	isDirty = true;
}

void GameObject2D::setPos(glm::vec2 newPos) {
	position = newPos;
	center = position + halfExtents;
	isDirty = true;
}

void GameObject2D::setPos(float newX, float newY) {
	position.x = newX;
	position.y = newY;
	center = position + halfExtents;
	isDirty = true;
}

void GameObject2D::translate(glm::vec2 posOff) {
	position += posOff;
	center += posOff;
	isDirty = true;
}

void GameObject2D::translate(float xOff, float yOff) {
	position.x += xOff;
	position.y += yOff;
	center.x += xOff;
	center.y += yOff;
	isDirty = true;
}

void GameObject2D::setRot(float newRot) {
	rotation = newRot;
	isDirty = true;
}

void GameObject2D::rotate(float rotOff) {
	rotation += rotOff;
	isDirty = true;
}

void GameObject2D::setScale(glm::vec2 newScale) {
	scaleVal = newScale;
	recalculateHalfExtents();
	isDirty = true;
}

void GameObject2D::setScale(float newX, float newY) {
	scaleVal.x = newX;
	scaleVal.y = newY;
	recalculateHalfExtents();
	isDirty = true;
}

void GameObject2D::scale(glm::vec2 scaleOff) {
	scaleVal += scaleOff;
	recalculateHalfExtents();
	isDirty = true;
}

void GameObject2D::scale(float xOff, float yOff) {
	scaleVal.x += xOff;
	scaleVal.y += yOff;
	recalculateHalfExtents();
	isDirty = true;
}

bool GameObject2D::inScreenBounds() {
	glm::vec2 appliedScale(scaleVal.x * sprite.width, scaleVal.y * sprite.height);
	return ((position.y + appliedScale.y < 0 || position.y >= SCR_HEIGHT) || (position.x + appliedScale.x < 0 || position.x >= SCR_WIDTH));
}

bool GameObject2D::collidesWith(GameObject2D* other) {
	return collider && other->collider ? collider->collision(center, rotation, other->collider, other->center, other->rotation) : false;
}

void GameObject2D::recalculateModel() {
	// Prepare transformations
	model = glm::mat4(1.0f);
	// round position in an effort to achieve pixel perfect 2D rendering
	model = glm::translate(model, glm::vec3(round(position.x), round(position.y), depth));

	// multiply scaling factor by sprite dimensions so that a scale of 1,1 = original size
	glm::vec2 appliedScale(scaleVal.x * sprite.width, scaleVal.y * sprite.height);

	// translate by the half extents to achieve centered rotation
	if (rotation != 0) {
		model = glm::translate(model, glm::vec3(0.5f * appliedScale.x, 0.5f * appliedScale.y, 0.0f));
		model = glm::rotate(model, rotation, glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::translate(model, glm::vec3(-0.5f * appliedScale.x, -0.5f * appliedScale.y, 0.0f));
	}

	model = glm::scale(model, glm::vec3(appliedScale, 1.0f));

	isDirty = false;
}