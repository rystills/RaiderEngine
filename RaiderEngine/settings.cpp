#include "stdafx.h"
#include "settings.hpp"
#include "camera.hpp"
#include "ObjectRegistryBase.hpp"
#include "model.hpp"
#include "GameObject.hpp"
#include "GameObject2D.hpp"
#include "Light.hpp"
#include "TextObject.hpp"
#include "physics.hpp"
#include "input.hpp"
#include "ParticleEmitter2D.hpp"
#include "audio.hpp"

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

Tilemap* addTilemap(Tilemap* go) {
	tilemaps.emplace_back(go);
	return go;
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

Collider2D* addCollider2D(std::string name, Collider2D* go) {
	colliders.insert(std::make_pair(name, go));
	return go;
}

ParticleEmitter2D* addParticleEmitter2D(ParticleEmitter2D* go) {
	particleEmitter2Ds.emplace_back(go);
	return go;
	// TODO: define remove methods for ParticleEmitter2D, Collider2D, and Tilemap 
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

void setEnableCursor(bool shouldEnable) {
	if (enableCursor != shouldEnable) {
		enableCursor = shouldEnable;
		glfwSetInputMode(window, GLFW_CURSOR, enableCursor ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
	}
}

void setScreenDimensions(int width, int height) {
	frameBufferSizeCallback(window, width, height);
}

void setScreenDimensions(glm::vec2 res) {
	frameBufferSizeCallback(window, static_cast<int>(res.x), static_cast<int>(res.y));
}

void setClearColor(glm::vec4 newColor) {
	clearColor = newColor;
	glClearColor(newColor.r, newColor.g, newColor.b, newColor.a);
}
void setClearColor(float r, float g, float b, float a) {
	clearColor = glm::vec4(r,g,b,a);
	glClearColor(r,g,b,a);
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

/*
update all objects in all object lists
*/
void updateObjects() {
	for (unsigned int i = 0; i < tilemaps.size(); ++i)
		tilemaps[i]->update();
	for (auto&& kv : gameObjects)
		for (unsigned int i = 0; i < kv.second.size(); ++i)
			kv.second[i]->update();
	for (unsigned int i = 0; i < lights.size(); ++i)
		lights[i]->update();
	for (auto&& kv : gameObject2Ds)
		for (unsigned int i = 0; i < kv.second.size(); ++i)
			kv.second[i]->update();
	for (unsigned int i = 0; i < particleEmitter2Ds.size(); ++i)
		particleEmitter2Ds[i]->update();
	for (unsigned int i = 0; i < textObjects.size(); ++i)
		textObjects[i]->update();
}

void initEngine() {
	initGraphics();
	initAudio();
}

void closeEngine() {
	closeGraphics();
	closeAudio();
	// clear the physics scene
	for (auto&& kv : gameObjects)
		for (unsigned int i = 0; i < kv.second.size(); ++i)
			if (kv.second[i]->usePhysics)
				gScene->removeActor(*kv.second[i]->body);
	cleanupPhysics();
	gameObjects.clear();
	gameObject2Ds.clear();
	lights.clear();
	fonts.clear();
	particleEmitter2Ds.clear();
	shaders.clear();
	colliders.clear();
	textObjects.clear();
	models.clear();
	sounds.clear();
}