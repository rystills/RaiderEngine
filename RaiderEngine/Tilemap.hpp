#pragma once
#include "stdafx.h"
#include "shader.hpp"
#include "mesh.hpp"
#include "Collider2D.hpp"

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
	std::vector<Collider2D*> tileColliders;

	Tilemap(std::string spriteName, int numTileTypes, int gridSize, glm::vec2 mapSize, glm::vec2 pos, std::vector<Collider2D*> cols, float depth = 0.99f);
	Tilemap(std::string spriteName, int numTileTypes, int gridSize, std::vector<std::vector<unsigned int>> map, glm::vec2 pos, std::vector<Collider2D*> cols, float depth = -.95f);

	void setTileData(GLfloat start[], int x, int y);

	virtual void update();

private:
	void init(std::string spriteName, std::vector<Collider2D*> cols);
};