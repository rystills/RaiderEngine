#pragma once
#include "stdafx.h"
#include "mesh.hpp"

struct Particle2D {
	glm::vec4 spriteColor;
	glm::vec2 pos;
	float scale;

	Particle2D(glm::vec4 spriteColor, glm::vec2 pos, float scale) : spriteColor(spriteColor), pos(pos), scale(scale) {}
};

struct ParticleMotionData2D {
	float speed, ang, life, maxLife, maxScale, maxAlpha;
	ParticleMotionData2D(float speed, float ang, float life, float maxScale, float maxAlpha) : speed(speed), ang(ang), life(life), maxLife(life), maxScale(maxScale), maxAlpha(maxAlpha) {}
};

class ParticleEmitter2D {
public:
	Texture sprite;
	inline static GLuint VAO, VBO;
	inline static unsigned int numParticlesInVBO = 0;
	bool randomSpawnTime = false;
	float partSpawnTimer = .01f, partSpawnMaxTimer = .01f, partSpawnMinTimer = .01f;
	std::vector<Particle2D> particles;
	std::vector<ParticleMotionData2D> particleMotions;
	std::vector<int> recycledParticleInds;
	glm::vec2 pos;
	float spawnRMin = 0, spawnRMax = 1, spawnGMin = 0, spawnGMax = 1, spawnBMin = 0, spawnBMax = 1, spawnAMin = 1, spawnAMax = 1;
	float spawnXOffMin = 0, spawnXOffMax = 0, spawnYOffMin = 0, spawnYOffMax = 0;
	float spawnScaleMin = 1, spawnScaleMax = 1;
	float spawnSpeedMin = 20, spawnSpeedMax = 30;
	float spawnAngleMin = 0, spawnAngleMax = 0;
	float spawnMinLife = 1, spawnMaxLife = 1;
	bool shrink = true;
	bool fade = true;
	bool circleSpawn = true;
	bool colorShift = false;
	bool isburst = false;
	bool recycleParticles = true;
	glm::vec3 colorShiftRate;
	glm::vec2 spawnOffset;  // use this to effectively move an emitter without displacing already active particles
	float depth = 0.1f;  // depth in NDC coordinates

	ParticleEmitter2D(glm::vec2 pos, std::string spriteName = "");

	static void initVertexObjects();

	void spawnParticle();

	bool killParticle(int partInd);

	virtual void update();
};