#include "stdafx.h"
#include "GameObject2D.hpp"
#include "model.hpp"
#include "settings.hpp"

GameObject2D::GameObject2D(glm::vec2 position, float rotation, glm::vec2 scale, glm::vec3 color, std::string spriteName, bool posIsCenter, float depth, Collider2D* collider) : 
	position(position), rotation(rotation), color(color), scale(scale), depth(depth), collider(collider) {
	sprite = (spriteName == "" ? Model::defaultDiffuseMap : Model::loadTextureSimple(spriteName));
	if (posIsCenter) {
		glm::vec2 halfExtents(scale.x * sprite.width * .5f, scale.y * sprite.height * .5f);
		position -= halfExtents;
	}
}

void GameObject2D::initStaticVertexBuffer() {
	VAO = std::move(std::unique_ptr < ALuint, std::function<void(ALuint*)>>{ new ALuint(0), std::bind(&deleteGraphicsBuffer, std::placeholders::_1) });
	// Configure VAO and temporary VBO
	GLuint VBO;
	GLfloat vertices[] = {
		// Pos      // Tex
		0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
	};

	glGenVertexArrays(1, VAO.get());
	glGenBuffers(1, &VBO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindVertexArray(*VAO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void GameObject2D::setCenter(glm::vec2 newCenter) {
	glm::vec2 halfExtents(scale.x * sprite.width * .5f, scale.y * sprite.height * .5f);
	position = newCenter - halfExtents;
}

bool GameObject2D::inScreenBounds() {
	glm::vec2 appliedScale(scale.x * sprite.width, scale.y * sprite.height);
	return ((position.y + appliedScale.y < 0 || position.y >= SCR_HEIGHT) || (position.x + appliedScale.x < 0 || position.x >= SCR_WIDTH));
}

glm::vec2 GameObject2D::center() {
	glm::vec2 halfExtents(scale.x * sprite.width * .5f, scale.y * sprite.height * .5f);
	return position + halfExtents;
}

bool GameObject2D::collidesWith(GameObject2D* other) {
	return collider && other->collider ? collider->collision(center(), rotation, other->collider, other->center(), other->rotation) : false;
}

void GameObject2D::draw(Shader shader, bool shouldSendTextures) {
	// don't draw blank sprites
	if (sprite.id == Model::defaultDiffuseMap.id)
		return;
	// Prepare transformations
	glm::mat4 model = glm::mat4(1.0f);
	// round position in an effort to achieve pixel perfect 2D rendering
	model = glm::translate(model, glm::vec3(round(position.x), round(position.y), depth));

	// multiply scaling factor by sprite dimensions so that a scale of 1,1 = original size
	glm::vec2 appliedScale(scale.x * sprite.width, scale.y * sprite.height);

	// translate by the half extents to achieve centered rotation
	if (rotation != 0) {
		model = glm::translate(model, glm::vec3(0.5f * appliedScale.x, 0.5f * appliedScale.y, 0.0f));
		model = glm::rotate(model, rotation, glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::translate(model, glm::vec3(-0.5f * appliedScale.x, -0.5f * appliedScale.y, 0.0f));
	}

	model = glm::scale(model, glm::vec3(appliedScale, 1.0f));

	shader.setMat4("model", model);
	shader.setVec3("spriteColor", color);

	if (shouldSendTextures) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, sprite.id);
	}
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}