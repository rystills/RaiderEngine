#pragma once
#include "shader.hpp"
#include "mesh.hpp"

class GameObject2D {
public:
	glm::vec2 position;  // topleft position
	float rotation;  // rotation measured in radians
	glm::vec2 scale;
	glm::vec3 color;
	Texture sprite;
	float depth;  // depth in NDC coordinates
	inline static std::unique_ptr<ALuint, std::function<void(ALuint*)>> VAO;

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
	GameObject2D(glm::vec2 position, float rotation, glm::vec2 scale, glm::vec3 color, std::string spriteName, bool posIsCenter = false, float depth = 0);

	/*
	initialize the VAO which defines the quad used when rendering any GameObject2D; this need only be called once at game start
	*/
	static void initStaticVertexBuffer();

	/*
	update the GameObject instance
	*/
	virtual void update() {};

	/*
	set this object's center coordinates
	@param newCenter: the position to which to set this object's center position
	*/
	void setCenter(glm::vec2 newCenter);

	/*
	check whether or not any part of this sprite is inside of the current screen boundaries
	@returns: whether or not any part of this sprite is within the screen bounds
	*/
	bool inScreenBounds();

	/*
	retrieve this object's center coordinates
	*/
	glm::vec2 center();

	/*
	draw this GameObject2D's sprite using the specified shader
	@param shader: the shader to use while drawing the sprite
	@param shouldSendTextures: whether or not to bind the sprite before rendering
	*/
	virtual void draw(Shader shader, bool shouldSendTextures = true);
};