#include "stdafx.h"
#pragma once
// this file holds global settings and shared engine data
unsigned int SCR_WIDTH = 1280, SCR_HEIGHT = 720;
bool useVsync = false;
bool fullScreen = false;
const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
bool debugDraw = false;

// TODO: move these vars into input.hpp
enum keyState { pressed = 0, held = 1, released = 2 };
int keyStates[GLFW_KEY_LAST][3] = { 0 };

std::string mapDir = "", modelDir = "", textureDir = "", soundDir = "", fontDir = "";

GLFWwindow* window;
#include "camera.hpp"
Camera* mainCam;
// TODO: we should not force usage of the Player base class
#include "ObjectRegistryBase.hpp"
ObjectRegistryBase* objectRegistry;

#include "model.hpp"
#include "GameObject.hpp"
#include "GameObject2D.hpp"
#include "Light.hpp"
#include "TextObject.hpp"
std::unordered_map<std::string, std::shared_ptr<Model>> models;
std::unordered_map<std::string, std::shared_ptr<ALuint>> sounds;
// GameObjects are indexed by their model name when created, allowing for automatic optimizations
std::unordered_map<std::string, std::vector<std::unique_ptr<GameObject>>> gameObjects;
std::unordered_map<std::string, std::vector<std::unique_ptr<GameObject2D>>> gameObject2Ds;
std::vector<std::unique_ptr<Light>> lights;
std::vector<std::unique_ptr<TextObject>> textObjects;
std::unordered_map<std::string,std::unique_ptr<Shader>> shaders;

void setMapDir(std::string newDir) {
	mapDir = newDir + (newDir.ends_with('/') ? "" : "/");
}
void setModelDir(std::string newDir) {
	modelDir = newDir + (newDir.ends_with('/') ? "" : "/");
}

void setTextureDir(std::string newDir) {
	textureDir = newDir + (newDir.ends_with('/') ? "" : "/");
}

void setSoundDir(std::string newDir) {
	soundDir = newDir + (newDir.ends_with('/') ? "" : "/");
}

void setFontDir(std::string newDir) {
	fontDir = newDir + (newDir.ends_with('/') ? "" : "/");
}

void setShaderDir(std::string newDir) {
	Shader::shaderDir = newDir + (newDir.ends_with('/') ? "" : "/");
}

/*
set the fallback directory from which to load shaders; useful when testing in an IDE where the working directory may not match the executable folder
*/
void setFallbackShaderDir(std::string newDir) {
	Shader::fallbackShaderDir = newDir + (newDir.ends_with('/') ? "" : "/");
}