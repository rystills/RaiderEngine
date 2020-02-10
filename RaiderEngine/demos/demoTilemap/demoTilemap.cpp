#include "stdafx.h"
#if (COMPILE_DEMO == DEMO_TILEMAP)
#include "GameObject2D.hpp"
#include "settings.hpp"
#include "timing.hpp"
#include "audio.hpp"
#include "FpsDisplay.hpp"
#include "input.hpp"
#include "Tilemap.hpp"
#include "input.hpp"

Tilemap* t;

void saveMap() {
	std::fstream mapFile;
	mapFile.open("demos/demoTilemap/map.txt", std::fstream::out | std::fstream::trunc);
	for (int i = 0; i < t->mapSize.x; ++i)
		for (int r = 0; r < t->mapSize.y; ++r)
			mapFile << t->map[i][r] << ' ';
	mapFile.close();
}

void loadMap() {
	std::fstream mapFile;
	mapFile.open("demos/demoTilemap/map.txt", std::fstream::in);
	if (mapFile.is_open()) {
		for (int i = 0; i < t->mapSize.x; ++i) {
			for (int r = 0; r < t->mapSize.y; ++r) {
				// TODO: quick and dirty saving / loading
				mapFile >> t->map[i][r];

				GLfloat verts[24];
				t->setTileData(&verts[0], i, r);

				glBindVertexArray(t->VAO);
				glBindBuffer(GL_ARRAY_BUFFER, t->VBO);
				glBufferSubData(GL_ARRAY_BUFFER, static_cast<GLintptr>(24 * sizeof(GLfloat) * (i * t->mapSize.y + r)), 24 * sizeof(GLfloat), verts);
			}
		}
		mapFile.close();
	}
}

int main() {
	initEngine();
	setEnableCursor(true);
	// directories
	setFontDir("demos/shared/fonts");
	setTextureDir("demos/demoTilemap/images");

	// keybindings
	setKeyBinding("saveMap", GLFW_KEY_S);
	setKeyBinding("loadMap", GLFW_KEY_L);

	// fonts
	freetypeLoadFont("Inter-Regular", 18);

	// TODO: pass map data directly into tilemap constructor
	t = addTilemap(new Tilemap("tilemap.png",64,glm::vec2(16,10),glm::vec2(0),3));
	loadMap();

	setVsync(false);
	setClearColor(.85f, .85f, 1.f, 1.f);
	addTextObject(new FpsDisplay(6, static_cast<float>(SCR_HEIGHT - 20), glm::vec3(0.f), "Inter-Regular", 18));
	addTextObject(new TextObject("Press S/L to save/load the Tilemap from the disk", 6, SCR_HEIGHT - 44, glm::vec3(0.f), "Inter-Regular", 18));
	addTextObject(new TextObject("Left click the grid spaces to cycle their tile types", 6, SCR_HEIGHT - 68, glm::vec3(0.f), "Inter-Regular", 18));

	while (beginFrame(false)) {
		updateObjects();
		// cycle hovered tile on left click
		if (mousePressedLeft) {
			int gridx = static_cast<int>((lastX - t->pos.x) / t->gridSize);
			int gridy = static_cast<int>((lastY - t->pos.y) / t->gridSize);
			if (!(gridx < 0 || gridy < 0 || gridx >= t->mapSize.x || gridy >= t->mapSize.y)) {
				(++t->map[gridx][gridy]) %= t->numTileTypes;

				GLfloat verts[24];
				t->setTileData(&verts[0], gridx, gridy);

				glBindVertexArray(t->VAO);
				glBindBuffer(GL_ARRAY_BUFFER, t->VBO);
				glBufferSubData(GL_ARRAY_BUFFER, static_cast<GLintptr>(24 * sizeof(GLfloat) * (gridx * t->mapSize.y + gridy)), 24 * sizeof(GLfloat), verts);
			}
		}

		// save on 'S' key press
		if (keyPressed("saveMap"))
			saveMap();

		// load on 'L' key press
		if (keyPressed("loadMap"))
			loadMap();

		render(true);
		// set the close flag if the player presses the escape key
		if (keyPressed(GLFW_KEY_ESCAPE))
			glfwSetWindowShouldClose(window, true);
	}
	closeEngine();
}
#endif