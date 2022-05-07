#include "stdafx.h"
#if (COMPILE_DEMO == DEMO_2D_PLATFORMER)
#include "GameObject2D.hpp"
#include "settings.hpp"
#include "timing.hpp"
#include "audio.hpp"
#include "FpsDisplay.hpp"
#include "input.hpp"
#include "Tilemap.hpp"
#include "input.hpp"
#include "Player.hpp"
#include "Collider2DRectangle.hpp"
#include <constants.hpp>

Tilemap* t;
const int numTileTypes = 3;

void saveMap() {
	std::fstream mapFile;
	mapFile.open("demos/demo2DPlatformer/map.txt", std::fstream::out | std::fstream::trunc);
	mapFile << static_cast<int>(t->mapSize.x) << ' ' << static_cast<int>(t->mapSize.y) << ' ';
	for (int i = 0; i < t->mapSize.x; ++i)
		for (int r = 0; r < t->mapSize.y; ++r)
			mapFile << t->map[i][r] << ' ';
	mapFile.close();
}

void reloadMap() {
	std::fstream mapFile;
	mapFile.open("demos/demo2DPlatformer/map.txt", std::fstream::in);
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
				t->setTileData(&verts[static_cast<int>(24 * (i * t->mapSize.y + r))], i, r);
			}
		}
		mapFile.close();
		glBufferSubData(GL_ARRAY_BUFFER, 0, static_cast<GLintptr>(24 * sizeof(GLfloat) * t->mapSize.x * t->mapSize.y), &verts[0]);
	}
}

int main() {
	// directories
	setFontDir("demos/shared/fonts");
	setTextureDir("demos/demo2DPlatformer/images");

	// keybindings
	setKeyBinding("mvLeft", GLFW_KEY_A);
	setKeyBinding("mvRight", GLFW_KEY_D);
	setKeyBinding("mvUp", GLFW_KEY_W);
	setKeyBinding("mvDown", GLFW_KEY_S);
	setKeyBinding("jump", GLFW_KEY_B);
	setKeyBinding("climb", GLFW_KEY_N);
	setKeyBinding("saveMap", GLFW_KEY_O);
	setKeyBinding("loadMap", GLFW_KEY_L);

	TARGET_WIDTH = 320;
	TARGET_HEIGHT = 180;
	SCR_WIDTH = 1280;
	SCR_HEIGHT = 720;
	UI_TARGET_WIDTH = 1920;
	UI_TARGET_HEIGHT = 1080;
	filterMin2D = GL_NEAREST_MIPMAP_NEAREST;
	filterMax2D = GL_NEAREST;
	initEngine();
	setEnableCursor(true);

	// fonts
	freetypeLoadFont("Inter-Regular", 24);

	// tile colliders
	Collider2D* tileCol = addCollider2D("8x8TileCollider", new Collider2DRectangle(4, 4));
	std::vector<Collider2D*> tileCols{ NULL,tileCol,tileCol, NULL };

	// attempt to load in the saved map data and pass it directly into the tilemap constructor
	std::fstream mapFile;
	mapFile.open("demos/demo2DPlatformer/map.txt", std::fstream::in);
	if (mapFile.is_open()) {
		int mapWidth, mapHeight;
		mapFile >> mapWidth >> mapHeight;
		std::vector<std::vector<unsigned int>> mapData(mapWidth, std::vector<unsigned int>(mapHeight));
		for (int i = 0; i < mapWidth; ++i)
			for (int r = 0; r < mapHeight; ++r)
				mapFile >> mapData[i][r];
		mapFile.close();
		t = addTilemap(new Tilemap("tilemap.png", 4, 8, mapData, glm::vec2(0,-2), tileCols));
	}
	else
		// no map data found; create an empty tilemap with the desired dimensions
		t = addTilemap(new Tilemap("tilemap.png", 4, 8, glm::vec2(40, 23), glm::vec2(0,-2), tileCols));

	setVsync(true);
	setClearColor(0,0,0, 1.f);
	addTextObject(new FpsDisplay(6, static_cast<float>(UI_TARGET_HEIGHT - 20), Color::white, "Inter-Regular", 24));
	addTextObject(new TextObject("Press O/L to save/load the Tilemap from the disk", 6, static_cast<float>(UI_TARGET_HEIGHT - 50), Color::magenta, "Inter-Regular", 24));
	addTextObject(new TextObject("Left click the grid spaces to cycle their tile types", 6, static_cast<float>(UI_TARGET_HEIGHT - 80), Color::magenta, "Inter-Regular", 24));
	addTextObject(new TextObject("Press A/D to move the player, B to jump, and N + W/S to climb", 6, static_cast<float>(UI_TARGET_HEIGHT - 110), Color::magenta, "Inter-Regular", 24));
	Player* player = (Player*)addGameObject2D(new Player(glm::vec2(8.f,TARGET_HEIGHT-22.f)));

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
	}
	closeEngine();
}
#endif