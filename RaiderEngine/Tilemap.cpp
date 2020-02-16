#include "stdafx.h"
#include "Tilemap.hpp"
#include "model.hpp"
#include "settings.hpp"
#include "timing.hpp"

void Tilemap::setTileData(GLfloat start[], int x, int y) {
	// TODO: simplify usage
	// code modified from graphics.cpp >> renderText
	GLfloat xpos = static_cast<GLfloat>(x * gridSize);
	GLfloat ypos = static_cast<GLfloat>(y * gridSize);
	GLfloat x0 = map[x][y] % (sprite.width / gridSize) / static_cast<float>((sprite.width / gridSize));
	GLfloat y0 = map[x][y] / (sprite.width / gridSize) / static_cast<float>((sprite.height / gridSize));

	GLfloat x1 = x0 + gridSize / static_cast<float>(sprite.width);
	GLfloat y1 = y0 + gridSize / static_cast<float>(sprite.height);

	// TODO: try tristrips (see GameObject2D / ParticleEmitter2D) for a minor performance boost
	// add image position and uv data to the vertex vector
	start[0] = xpos;
	start[1] = ypos;
	start[2] = x0;
	start[3] = y0;
	
	start[4] = xpos;
	start[5] = ypos + gridSize;
	start[6] = x0;
	start[7] = y1;
	
	start[8] = xpos + gridSize;
	start[9] = ypos + gridSize;
	start[10] = x1;
	start[11] = y1;
	
	start[12] = xpos;
	start[13] = ypos;
	start[14] = x0;
	start[15] = y0;
	
	start[16] = xpos + gridSize;
	start[17] = ypos + gridSize;
	start[18] = x1;
	start[19] = y1;
	
	start[20] = xpos + gridSize;
	start[21] = ypos;
	start[22] = x1;
	start[23] = y0;
}

void Tilemap::init(std::string spriteName) {
	sprite = (spriteName == "" ? Model::defaultDiffuseMap : Model::loadTextureSimple(spriteName));
	// TODO: consider instanced rendering visible tiles rather than storing and rendering entire tilemap every frame
	// init VAO/VBO
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);

	std::vector<GLfloat> verts;
	verts.reserve(static_cast<unsigned int>(24 * mapSize.x * mapSize.y));
	// set each tile
	for (unsigned int i = 0; i < mapSize.x; ++i)
		for (unsigned int r = 0; r < mapSize.y; ++r)
			setTileData(&verts[0] + static_cast<int>(24 * (i * mapSize.y + r)), i, r);

	// buffer the full set of tiles as a single triangle array for rendering (use GL_STATIC_DRAW as Tilemaps are not expected to change frequently)
	glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(24 * sizeof(GLfloat) * mapSize.x * mapSize.y), &verts[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// fill the Tilemap with NULL colliders initially  
	// TODO: pass colliders into tilemap constructor
	tileColliders.resize(numTileTypes, NULL);

}

Tilemap::Tilemap(std::string spriteName, int numTileTypes, int gridSize, glm::vec2 mapSize, glm::vec2 pos, float depth) : numTileTypes(numTileTypes), gridSize(gridSize), mapSize(mapSize), pos(pos), depth(depth) {
	// init empty map, since none was provided
	map.resize(static_cast<unsigned int>(mapSize.x));
	for (unsigned int i = 0; i < mapSize.x; ++i)
		map[i].resize(static_cast<unsigned int>(mapSize.y));
	init(spriteName);
}

Tilemap::Tilemap(std::string spriteName, int numTileTypes, int gridSize, std::vector<std::vector<unsigned int>> map, glm::vec2 pos, float depth) : numTileTypes(numTileTypes), gridSize(gridSize), map(map), pos(pos), depth(depth) {
	// infer mapSize from provided map
	mapSize = glm::vec2(map.size(), map.size() > 0 ? map[0].size() : 0);
	init(spriteName);
}

void Tilemap::update() {
}