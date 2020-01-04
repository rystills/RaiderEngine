#pragma once
#include "stdafx.h"
#include "mesh.hpp"
#include "model.hpp"

class GameObject2D {
public:
	glm::vec2 position;  // topleft position
	float rotation;  // rotation measured in radians
	glm::vec2 scale;
	glm::vec3 color;
	Texture sprite;
	float depth;  // depth in NDC coordinates
	inline static std::shared_ptr<unsigned int> VAO = std::shared_ptr<unsigned int>(new unsigned int(0), deleteGraphicsBuffer);

	/*
	GameObject2D constructor: creates a new GameObject2D with the specified transforms and sprite
	@param position: the GameObject2D's initial position
	@param rotation: the GameObject2D's initial rotation (in radians)
	@param scale: the GameObject2D's initial scale, with 0-1 being equivalent to 0-100% of the original size
	@param color: the color by which to multiply the sprite, with each channel in the range of 0-1; set to white (1,1,1) for no color modification
	@param spriteName: the name of the sprite that this GameObject should use; will attempt to load the sprite if it is not present in the image map
	@param posIsCenter: whether the initial position marks the center of the sprite, or the topleft corner of the sprite
	@param depth: the GameObject2D's depth in NDC coordinates; this means the range is [-1,1], with larger values appearing closer to the screen (rendering in front)
	*/
	GameObject2D(glm::vec2 position, float rotation, glm::vec2 scale, glm::vec3 color, std::string spriteName, bool posIsCenter = false, float depth = 0) : position(position), rotation(rotation), color(color), scale(scale), depth(depth) {
		sprite = Model::loadTextureSimple(spriteName);
		if (posIsCenter) {
			glm::vec2 halfExtents(scale.x * sprite.width * .5f, scale.y * sprite.height * .5f);
			position -= halfExtents;
		}
	}

	/*
	initialize the VAO which defines the quad used when rendering any GameObject2D; this need only be called once at game start
	*/
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
	set this object's center coordinates
	@param newCenter: the position to which to set this object's center position
	*/
	void setCenter(glm::vec2 newCenter) {
		glm::vec2 halfExtents(scale.x * sprite.width * .5f, scale.y * sprite.height * .5f);
		position = newCenter - halfExtents;
	}

	/*
	check whether or not any part of this sprite is inside of the current screen boundaries
	@returns: whether or not any part of this sprite is within the screen bounds
	*/
	bool inScreenBounds() {
		glm::vec2 appliedScale(scale.x * sprite.width, scale.y * sprite.height);
		return ((position.y + appliedScale.y < 0 || position.y >= SCR_HEIGHT) || (position.x + appliedScale.x < 0 || position.x >= SCR_WIDTH));
	}

	/*
	retrieve this object's center coordinates
	*/
	glm::vec2 center() {
		glm::vec2 halfExtents(scale.x * sprite.width * .5f, scale.y * sprite.height * .5f);
		return position + halfExtents;
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
		model = glm::translate(model, glm::vec3(position, depth));

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