#pragma once
#include "stdafx.h"
#include "mesh.hpp"

struct Particle {
	glm::vec4 spriteColor;
	glm::vec3 pos;
	float scale;

	Particle(glm::vec4 spriteColor, glm::vec3 pos, float scale) : spriteColor(spriteColor), pos(pos), scale(scale) {}
};

struct ParticleMotionData {
	float speed, pitch, yaw, life, maxLife, maxScale, maxAlpha;
	ParticleMotionData(float speed, float pitch, float yaw, float life, float maxScale, float maxAlpha) : speed(speed), pitch(pitch), yaw(yaw), life(life), maxLife(life), maxScale(maxScale), maxAlpha(maxAlpha) {}
};

class ParticleEmitter {
public: 
	Texture sprite;
	inline static GLuint VAO, VBO;
	inline static unsigned int numParticlesInVBO = 0;
	bool randomSpawnTime = false;
	float partSpawnTimer = .01f, partSpawnMaxTimer = .01f, partSpawnMinTimer = .01f;
	std::vector<Particle> particles;
	std::vector<ParticleMotionData> particleMotions;
	std::vector<int> recycledParticleInds;
	glm::vec3 pos;
	float spawnRMin = 0, spawnRMax = 1, spawnGMin = 0, spawnGMax = 1, spawnBMin = 0, spawnBMax = 1, spawnAMin = 1, spawnAMax = 1;
	float spawnXOffMin = 0, spawnXOffMax = 0, spawnYOffMin = 0, spawnYOffMax = 0, spawnZOffMin = 0, spawnZOffMax = 0;
	float spawnScaleMin = 1, spawnScaleMax = 1;
	float spawnSpeedMin = 20, spawnSpeedMax = 30;
	float spawnPitchMin = 0, spawnPitchMax = 0;
	float spawnYawMin = 0, spawnYawMax = 0;
	float spawnMinLife = 1, spawnMaxLife = 1;
	bool shrink = true;
	bool fade = true;
	bool sphereSpawn = true;
	bool colorShift = false;
	bool isburst = false;
	bool recycleParticles = true;
	glm::vec3 colorShiftRate;
	glm::vec3 spawnOffset;  // use this to effectively move an emitter without displacing already active particles

	ParticleEmitter(glm::vec3 pos, std::string spriteName = "");

	static void initVertexObjects();

	void spawnParticle();

	bool killParticle(int partInd);

	virtual void update();
};