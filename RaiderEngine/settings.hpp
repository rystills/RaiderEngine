#pragma once
#include "stdafx.h"
#include "camera.hpp"
#include "ObjectRegistryBase.hpp"
#include "model.hpp"
#include "GameObject.hpp"
#include "GameObject2D.hpp"
#include "Light.hpp"
#include "TextObject.hpp"
#include <graphics.hpp>
#include "ParticleEmitter2D.hpp"

// this file holds global settings and shared engine data
inline unsigned int SCR_WIDTH = 1280, SCR_HEIGHT = 720;
inline bool useVsync = false;
inline bool fullScreen = false;
inline const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
inline bool debugDraw = false;
inline float anisoFilterAmount = 0.0f;
inline const int numFontCharacters = 128;  // we only care about the first 128 characters stored in a given font file, at least for now

inline std::string mapDir = "", modelDir = "", textureDir = "", soundDir = "", fontDir = "";

inline GLFWwindow* window;

inline Camera* mainCam;

inline ObjectRegistryBase* objectRegistry;

inline std::unordered_map<std::string, std::unique_ptr<Model>> models;
inline std::unordered_map<std::string, std::unique_ptr<ALuint, std::function<void(ALuint*)>>> sounds;
// GameObjects are indexed by their model name when created, allowing for automatic optimizations
inline std::unordered_map<std::string, std::vector<std::unique_ptr<GameObject>>> gameObjects;
inline std::unordered_map<std::string, std::vector<std::unique_ptr<GameObject2D>>> gameObject2Ds;
inline std::vector<std::unique_ptr<Light>> lights;
inline std::unordered_map<std::string, std::unordered_map<int, std::pair< GLuint, Character[numFontCharacters]>>> fonts;
inline std::pair<std::string, int> lastFontUsed;
inline glm::vec3 lastFontColor;
inline std::vector<std::unique_ptr<TextObject>> textObjects;
inline std::unordered_map<std::string,std::unique_ptr<Shader>> shaders;
inline std::unordered_map<std::string, std::unique_ptr<Collider2D>> colliders;
inline std::vector<std::unique_ptr<ParticleEmitter2D>> particleEmitter2Ds;
inline bool forceDisableNvidiaThreadedOptimization = false;

/* methods to add/remove objects from the engine */
GameObject* addGameObject(GameObject* go);

void removeGameObject(GameObject* go);

void removeGameObject(std::string modelName, int ind);

GameObject2D* addGameObject2D(GameObject2D* go);

void removeGameObject2D(GameObject2D* go);

void removeGameObject2D(std::string spriteName, int ind);

Collider2D* addCollider2D(std::string name, Collider2D* go);

ParticleEmitter2D* addParticleEmitter2D(ParticleEmitter2D* go);

TextObject* addTextObject(TextObject* go);

void removeTextObject(TextObject* go);

void removeTextObject(int ind);

Light* addLight(Light* go);

void removeLight(Light* go);

void removeLight(int ind);

void setVsync(bool shouldUse);

void setScreenDimensions(int width, int height);
void setScreenDimensions(glm::vec2 res);

void setClearColor(glm::vec4 newColor);
void setClearColor(float r, float g, float b, float a);

void setMapDir(std::string newDir);

void setModelDir(std::string newDir);

void setTextureDir(std::string newDir);

void setSoundDir(std::string newDir);

void setFontDir(std::string newDir);

void setShaderDir(std::string newDir);

/*
set the fallback directory from which to load shaders; useful when testing in an IDE where the working directory may not match the executable folder
*/
void setFallbackShaderDir(std::string newDir);

/*
update all objects in all object lists
*/
void updateObjects();

void initEngine();

void closeEngine();