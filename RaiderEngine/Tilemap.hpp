#pragma once
#include "stdafx.h"
#include "shader.hpp"
#include "mesh.hpp"

class Tilemap {
public:
	GLuint VAO, VBO;
	Texture sprite;
	int gridSize;  // length (in pixels) of a single grid space
	glm::vec2 mapSize;  // total number of tiles horizontally and vertically
	std::vector<std::vector<unsigned int>> map;
	float depth;  // depth in NDC coordinates
	glm::vec2 pos;

	Tilemap(std::string spriteName, int gridSize, glm::vec2 mapSize, glm::vec2 pos, float depth = 0.99f);

	virtual void update();
};