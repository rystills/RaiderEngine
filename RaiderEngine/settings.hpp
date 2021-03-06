#pragma once
#include "stdafx.h"
#include "camera.hpp"
#include "ObjectRegistryBase.hpp"
#include "model.hpp"
#include "GameObject.hpp"
#include "GameObject2D.hpp"
#include "Light.hpp"
#include "TextObject.hpp"
#include "graphics.hpp"
#include "ParticleEmitter.hpp"
#include "ParticleEmitter2D.hpp"
#include "Tilemap.hpp"

// this file holds global settings and shared engine data
inline unsigned int SCR_WIDTH = 1280, SCR_HEIGHT = 720, TARGET_WIDTH = 1280, TARGET_HEIGHT = 720, UI_TARGET_WIDTH = 1280, UI_TARGET_HEIGHT = 720, MONITOR_WIDTH = 0, MONITOR_HEIGHT = 0, MONITOR_REFRESH_RATE = 0;
inline const unsigned int SHADOW_WIDTH = 512, SHADOW_HEIGHT = 512;
inline bool drawColliders = false;
inline bool drawNormals = false;
inline bool drawLightCubes = false;
inline bool drawLightSpheres = false;
// global visual debugging toggles
inline bool enableTextureMaps[Model::numMapTypes] = {true, true, true, true};
inline bool enableLighting = true;
inline bool freezeWorld = false;
inline float anisoFilterAmount = 0.0f;
inline GLuint filterMin2D = GL_LINEAR_MIPMAP_LINEAR, filterMax2D= GL_LINEAR;
inline GLuint wrapS2D = GL_CLAMP_TO_EDGE, wrapT2D = GL_CLAMP_TO_EDGE;

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
inline std::unordered_map<std::string, std::unordered_map<int, std::pair< GLuint, CharacterSheet>>> fonts;
inline std::vector<std::unique_ptr<TextObject>> textObjects;
inline std::unordered_map<std::string,std::unique_ptr<Shader>> shaders;
inline std::unordered_map<std::string, std::unique_ptr<Collider2D>> colliders;
inline std::vector<std::unique_ptr<ParticleEmitter>> particleEmitters;
inline std::vector<std::unique_ptr<ParticleEmitter2D>> particleEmitter2Ds;
inline std::vector<std::unique_ptr<Tilemap>> tilemaps;
inline bool forceDisableNvidiaThreadedOptimization = false;
inline bool ignoreModelTexturePaths = true;
inline bool clearEachFrame = true;

// openGL render state
struct RenderState {
	bool blend = false, depthTest = false;
	int blendFuncSrc, blendFuncDest;
	// TODO: all render state vars should likely be given a proper update method
	glm::mat4 projection, view;
	unsigned int numLights;
	glm::vec3 viewPos;
	float far_plane;
	float ambientStrength = .3f, prevAmbientStrength;
	glm::vec4 clearColor, prevClearColor;
	bool underWater = false;
	bool twoSidedDrawing = true;
	bool useVsync = false;
	bool enableCursor = false;
	bool fullScreen = false;
	std::pair<std::string, int> lastFontUsed;
	glm::vec3 lastFontColor;
	glm::vec2 uvScroll2D = glm::vec2(std::numeric_limits<float>::infinity());
} inline renderState;

/* methods to add/remove objects from the engine */
GameObject* addGameObject(GameObject* go);

void removeGameObject(GameObject* go);

void removeGameObject(std::string modelName, int ind);

Tilemap* addTilemap(Tilemap* go);

GameObject2D* addGameObject2D(GameObject2D* go);

void removeGameObject2D(GameObject2D* go);

void removeGameObject2D(std::string spriteName, int ind);

Collider2D* addCollider2D(std::string name, Collider2D* go);

ParticleEmitter* addParticleEmitter(ParticleEmitter* go);

ParticleEmitter2D* addParticleEmitter2D(ParticleEmitter2D* go);

TextObject* addTextObject(TextObject* go);

void removeTextObject(TextObject* go);

void removeTextObject(int ind);

Light* addLight(Light* go);

void removeLight(Light* go);

void removeLight(int ind);

void setVsync(bool shouldUse);

void setWindowMode(int newWidth, int newHeight, bool newFullScreen);

void setEnableCursor(bool shouldEnable);

void setScreenDimensions(int width, int height);
void setScreenDimensions(glm::vec2 res);

void setShouldBlend(bool blend);

void setShouldDepthTest(bool test);

void setBlendFunc(int funcSrc, int funcDest);

void setAmbientStrength(float as);

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