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
	int numTileTypes;

	Tilemap(std::string spriteName, int gridSize, glm::vec2 mapSize, glm::vec2 pos, int numTileTypes, float depth = 0.99f);

	void setTileData(GLfloat start[], int x, int y);

	virtual void update();
};