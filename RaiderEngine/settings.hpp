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
std::unordered_map<std::string, std::unique_ptr<Model>> models;
std::unordered_map<std::string, std::unique_ptr<ALuint, std::function<void(ALuint*)>>> sounds;
// GameObjects are indexed by their model name when created, allowing for automatic optimizations
std::unordered_map<std::string, std::vector<std::unique_ptr<GameObject>>> gameObjects;
std::unordered_map<std::string, std::vector<std::unique_ptr<GameObject2D>>> gameObject2Ds;
std::vector<std::unique_ptr<Light>> lights;
std::vector<std::unique_ptr<TextObject>> textObjects;
std::unordered_map<std::string,std::unique_ptr<Shader>> shaders;

/* methods to add/remove objects from the engine */
GameObject* addGameObject(GameObject* go) {
	gameObjects[go->modelName].emplace_back(go);
	return go;
}

void removeGameObject(GameObject* go) {
	if (go->usePhysics)
		gScene->removeActor(*go->body);
	gameObjects[go->modelName].erase(std::find_if(gameObjects[go->modelName].begin(), gameObjects[go->modelName].end(), [&](std::unique_ptr<GameObject>& i) { return i.get() == go; }));
}

void removeGameObject(std::string modelName, int ind) {
	if (gameObjects[modelName][ind]->usePhysics)
		gScene->removeActor(*gameObjects[modelName][ind]->body);
	gameObjects[modelName].erase(gameObjects[modelName].begin() + ind);
}

GameObject2D* addGameObject2D(GameObject2D* go) {
	gameObject2Ds[std::filesystem::path(go->sprite.path).stem().string()].emplace_back(go);
	return go;
}

void removeGameObject2D(GameObject2D* go) {
	std::string spriteName = std::filesystem::path(go->sprite.path).stem().string();
	gameObject2Ds[spriteName].erase(std::find_if(gameObject2Ds[spriteName].begin(), gameObject2Ds[spriteName].end(), [&](std::unique_ptr<GameObject2D>& i) { return i.get() == go; }));
}

void removeGameObject2D(std::string spriteName, int ind) {
	gameObject2Ds[spriteName].erase(gameObject2Ds[spriteName].begin() + ind);
}

TextObject* addTextObject(TextObject* go) {
	textObjects.emplace_back(go);
	return go;
}

void removeTextObject(TextObject* go) {
	textObjects.erase(std::find_if(textObjects.begin(), textObjects.end(), [&](std::unique_ptr<TextObject>& i) { return i.get() == go; }));
}

void removeTextObject(int ind) {
	textObjects.erase(textObjects.begin() + ind);
}

Light* addLight(Light* go) {
	lights.emplace_back(go);
	return go;
}

void removeLight(Light* go) {
	lights.erase(std::find_if(lights.begin(), lights.end(), [&](std::unique_ptr<Light>& i) { return i.get() == go; }));
}

void removeLight(int ind) {
	lights.erase(lights.begin() + ind);
}

void setVsync(bool shouldUse) {
	if (useVsync != shouldUse) {
		useVsync = shouldUse;
		glfwSwapInterval(useVsync);
	}
}

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