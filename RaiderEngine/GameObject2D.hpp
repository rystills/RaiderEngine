#pragma once
#include "stdafx.h"
#include "mesh.hpp"
#include "model.hpp"

class GameObject2D {
public:
	glm::vec2 position;
	float rotation;
	glm::vec2 scale;
	glm::vec3 color;
	Texture sprite;
	inline static std::shared_ptr<unsigned int> VAO = std::shared_ptr<unsigned int>(new unsigned int(0), deleteGraphicsBuffer);

	/*
	GameObject2D constructor: creates a new GameObject2D with the specified transforms and sprite
	@param
	*/
	GameObject2D(glm::vec2 position, float rotation, glm::vec2 scale, glm::vec3 color, std::string spriteName) : position(position), rotation(rotation), color(color), scale(scale) {
		sprite = Model::loadTextureSimple(spriteName);
	}

	static void initStaticVertexBuffer() {
		// Configure VAO and temporary VBO
		GLuint VBO;
		GLfloat vertices[] = {
			// Pos      // Tex
			0.0f, 1.0f, 0.0f, 1.0f,
			1.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 0.0f,

			0.0f, 1.0f, 0.0f, 1.0f,
			1.0f, 1.0f, 1.0f, 1.0f,
			1.0f, 0.0f, 1.0f, 0.0f
		};

		glGenVertexArrays(1, &*VAO);
		glGenBuffers(1, &VBO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glBindVertexArray(*VAO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	/*
	update the GameObject instance
	@param deltaTime: the elapsed time (in seconds) since the previous frame
	*/
	virtual void update(float deltaTime) {
	}

	/*
	draw this GameObject2D's sprite using the specified shader
	@param shader: the shader to use while drawing the sprite
	@param shouldSendTextures: whether or not to bind the sprite before rendering
	*/
	virtual void draw(Shader shader, bool shouldSendTextures = true) {
		// TODO: conditionally send texture based on provided flag for optimization when rendering many objects which share the same sprite

		// Prepare transformations
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(position, 0.0f));

		// multiply scaling factor by sprite dimensions so that a scale of 1,1 = original size
		glm::vec2 appliedScale(scale.x * sprite.width, scale.y * sprite.height);

		// translate by the half extents to achieve centered rotation
		model = glm::translate(model, glm::vec3(0.5f * appliedScale.x, 0.5f * appliedScale.y, 0.0f));
		model = glm::rotate(model, rotation, glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::translate(model, glm::vec3(-0.5f * appliedScale.x, -0.5f * appliedScale.y, 0.0f));

		model = glm::scale(model, glm::vec3(appliedScale, 1.0f));

		shader.setMat4("model", model);
		shader.setVec3("spriteColor", color);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, sprite.id);

		glBindVertexArray(*VAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
	}
};