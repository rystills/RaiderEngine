#include "stdafx.h"
#include "Tilemap.hpp"
#include "model.hpp"
#include "settings.hpp"
#include "timing.hpp"

Tilemap::Tilemap(std::string spriteName, int gridSize, glm::vec2 mapSize, glm::vec2 pos, float depth) : gridSize(gridSize), mapSize(mapSize), pos(pos), depth(depth) {
	sprite = (spriteName == "" ? Model::defaultDiffuseMap : Model::loadTextureSimple(spriteName));
	// TODO: consider instanced rendering visible tiles rather than storing and rendering entire tilemap every frame
	// init VAO/VBO
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);

	// randomize map  // TODO: placeholder
	map.resize(mapSize.x);
	for (unsigned int i = 0; i < mapSize.x; ++i) {
		map[i].reserve(mapSize.y);
		for (unsigned int r = 0; r < mapSize.y; ++r)
			map[i][r] = rand() % 4;
	}
	// code modified from graphics.cpp >> renderText
	std::vector<GLfloat> verts;
	verts.reserve(24 * mapSize.x * mapSize.y);
	// Iterate through all characters
	for (unsigned int i = 0; i < mapSize.x; ++i) {
		for (unsigned int r = 0; r < mapSize.y; ++r) {
			GLfloat xpos = i*gridSize;
			GLfloat ypos = r*gridSize;
			GLfloat x0 = map[i][r] % (sprite.width / gridSize) / static_cast<float>((sprite.width / gridSize));
			GLfloat y0 = map[i][r] / (sprite.width / gridSize) / static_cast<float>((sprite.height / gridSize));

			GLfloat x1 = x0 + gridSize / static_cast<float>(sprite.width);
			GLfloat y1 = y0 + gridSize / static_cast<float>(sprite.height);

			// TODO: try tristrips (see GameObject2D / ParticleEmitter2D) for a minor performance boost
			// add image position and uv data to the vertex vector
			verts[24 * (i*mapSize.y+r)] = xpos;
			verts[24 * (i*mapSize.y+r) + 1] = ypos;
			verts[24 * (i*mapSize.y+r) + 2] = x0;
			verts[24 * (i*mapSize.y+r) + 3] = y0;

			verts[24 * (i*mapSize.y+r) + 4] = xpos;
			verts[24 * (i*mapSize.y+r) + 5] = ypos + gridSize;
			verts[24 * (i*mapSize.y+r) + 6] = x0;
			verts[24 * (i*mapSize.y+r) + 7] = y1;

			verts[24 * (i*mapSize.y+r) + 8] = xpos + gridSize;
			verts[24 * (i*mapSize.y+r) + 9] = ypos + gridSize;
			verts[24 * (i*mapSize.y+r) + 10] = x1;
			verts[24 * (i*mapSize.y+r) + 11] = y1;

			verts[24 * (i*mapSize.y+r) + 12] = xpos;
			verts[24 * (i*mapSize.y+r) + 13] = ypos;
			verts[24 * (i*mapSize.y+r) + 14] = x0;
			verts[24 * (i*mapSize.y+r) + 15] = y0;

			verts[24 * (i*mapSize.y+r) + 16] = xpos + gridSize;
			verts[24 * (i*mapSize.y+r) + 17] = ypos + gridSize;
			verts[24 * (i*mapSize.y+r) + 18] = x1;
			verts[24 * (i*mapSize.y+r) + 19] = y1;

			verts[24 * (i*mapSize.y+r) + 20] = xpos + gridSize;
			verts[24 * (i*mapSize.y+r) + 21] = ypos;
			verts[24 * (i*mapSize.y+r) + 22] = x1;
			verts[24 * (i*mapSize.y+r) + 23] = y0;
		}
	}

	// buffer the full set of tiles as a single triangle array for rendering
	glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(GLfloat) * mapSize.x * mapSize.y, &verts[0], GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void Tilemap::update() {
}