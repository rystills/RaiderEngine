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
const int numTileTypes = 3;

void saveMap() {
	std::fstream mapFile;
	mapFile.open("demos/demoTilemap/map.txt", std::fstream::out | std::fstream::trunc);
	mapFile << static_cast<int>(t->mapSize.x) << ' ' << static_cast<int>(t->mapSize.y) << ' ';
	for (int i = 0; i < t->mapSize.x; ++i)
		for (int r = 0; r < t->mapSize.y; ++r)
			mapFile << t->map[i][r] << ' ';
	mapFile.close();
}

void reloadMap() {
	std::fstream mapFile;
	mapFile.open("demos/demoTilemap/map.txt", std::fstream::in);
	if (mapFile.is_open()) {
		glBindVertexArray(t->VAO);
		glBindBuffer(GL_ARRAY_BUFFER, t->VBO);
		std::vector<GLfloat> verts;
		verts.reserve(static_cast<unsigned int>(24 * t->mapSize.x * t->mapSize.y));
		// ignore map width and height, since we already have them stored in the Tilemap
		mapFile.ignore(std::numeric_limits<std::streamsize>::max(),' ').ignore(std::numeric_limits<std::streamsize>::max(), ' ');
		for (int i = 0; i < t->mapSize.x; ++i) {
			for (int r = 0; r < t->mapSize.y; ++r) {
				mapFile >> t->map[i][r];
				t->setTileData(&verts[0] + static_cast<int>(24 * (i * t->mapSize.y + r)), i, r);
			}
		}
		mapFile.close();
		glBufferSubData(GL_ARRAY_BUFFER, 0, static_cast<GLintptr>(24 * sizeof(GLfloat) * t->mapSize.x * t->mapSize.y), &verts[0]);
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

	// attempt to load in the saved map data and pass it directly into the tilemap constructor
	std::fstream mapFile;
	mapFile.open("demos/demoTilemap/map.txt", std::fstream::in);
	if (mapFile.is_open()) {
		int mapWidth, mapHeight;
		mapFile >> mapWidth >> mapHeight;
		std::vector<std::vector<unsigned int>> mapData(mapWidth, std::vector<unsigned int>(mapHeight));
		for (int i = 0; i < mapWidth; ++i)
			for (int r = 0; r < mapHeight; ++r)
				mapFile >> mapData[i][r];
		mapFile.close();
		t = addTilemap(new Tilemap("tilemap.png", 64, mapData, glm::vec2(0)));
	}
	else
		// no map data found; create an empty tilemap with the desired dimensions
		t = addTilemap(new Tilemap("tilemap.png", 64, glm::vec2(16, 10), glm::vec2(0)));

	setVsync(false);
	setClearColor(.85f, .85f, 1.f, 1.f);
	addTextObject(new FpsDisplay(6, static_cast<float>(TARGET_HEIGHT - 20), Color::black, "Inter-Regular", 18));
	addTextObject(new TextObject("Press S/L to save/load the Tilemap from the disk", 6, static_cast<float>(TARGET_HEIGHT - 44), Color::black, "Inter-Regular", 18));
	addTextObject(new TextObject("Left click the grid spaces to cycle their tile types", 6, static_cast<float>(TARGET_HEIGHT - 68), Color::black, "Inter-Regular", 18));

	int lastGridx = 0, lastGridy = 0;
	while (beginFrame(false)) {
		checkDemoToggles();
		updateObjects();
		// cycle hovered tile on left mouse
		if (mouseHeldLeft) {
			int gridx = static_cast<int>((lastX*(TARGET_WIDTH/static_cast<float>(SCR_WIDTH)) - t->pos.x) / t->gridSize);
			int gridy = static_cast<int>((lastY*(TARGET_WIDTH/static_cast<float>(SCR_WIDTH)) - t->pos.y) / t->gridSize);
			// allow clicking or dragging
			if (mousePressedLeft || gridx != lastGridx || gridy != lastGridy) {
				if (!(gridx < 0 || gridy < 0 || gridx >= t->mapSize.x || gridy >= t->mapSize.y)) {
					(++t->map[gridx][gridy]) %= numTileTypes;

					GLfloat verts[24];
					t->setTileData(&verts[0], gridx, gridy);

					glBindVertexArray(t->VAO);
					glBindBuffer(GL_ARRAY_BUFFER, t->VBO);
					glBufferSubData(GL_ARRAY_BUFFER, static_cast<GLintptr>(24 * sizeof(GLfloat) * (gridx * t->mapSize.y + gridy)), 24 * sizeof(GLfloat), verts);
				}
			}
			lastGridx = gridx, lastGridy = gridy;
		}

		// save on 'S' key press
		if (keyPressed("saveMap"))
			saveMap();

		// load on 'L' key press
		if (keyPressed("loadMap"))
			reloadMap();

		render(true);
		// set the close flag if the player presses the escape key
		if (keyPressed(GLFW_KEY_ESCAPE))
			glfwSetWindowShouldClose(window, true);
	}
	closeEngine();
}
#endif