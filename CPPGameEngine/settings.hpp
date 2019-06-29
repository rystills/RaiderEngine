#include "stdafx.h"
#pragma once
// this file holds global settings and shared engine data
unsigned int SCR_WIDTH = 1280, SCR_HEIGHT = 720;
bool useVsync = true;
bool fullScreen = false;
const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
bool debugDraw = false;

std::string mapDir = "", modelDir = "", textureDir = "", soundDir = "", fontDir = "";

GLFWwindow* window;
#include "Player.hpp"
// TODO: we should not force usage of the Player base class
Player player;

#include "model.hpp"
#include "GameObject.hpp"
#include "Light.hpp"
#include "TextObject.hpp"
std::unordered_map<std::string, std::shared_ptr<Model>> models;
std::unordered_map<std::string, std::shared_ptr<ALuint>> sounds;
std::vector<std::unique_ptr<GameObject>> gameObjects;
std::vector<std::unique_ptr<Light>> lights;
std::vector<std::unique_ptr<TextObject>> textObjects;
std::unordered_map<std::string,std::unique_ptr<Shader>> shaders;

void setMapDir(std::string newDir) {
	mapDir = newDir + '/';
}
void setModelDir(std::string newDir) {
	modelDir = newDir + '/';
}

void setTextureDir(std::string newDir) {
	textureDir = newDir + '/';
}

void setSoundDir(std::string newDir) {
	soundDir = newDir + '/';
}

void setFontDir(std::string newDir) {
	fontDir = newDir + '/';
}