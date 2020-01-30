#include "stdafx.h"
#include "ParticleEmitter2D.hpp"
#include "FireEmitter.hpp"
#include "input.hpp"

FireEmitter::FireEmitter(glm::vec2 pos) : ParticleEmitter2D(pos, "fire.png") {
	partSpawnMaxTimer = .001f;
	spawnRMin = .7f, spawnRMax = 1.f, spawnGMin = .1f, spawnGMax = .4f, spawnBMin = .05f, spawnBMax = .15f;
	spawnXOffMin = -20, spawnXOffMax = 20, spawnYOffMin = -5, spawnYOffMax = 5;
	spawnScaleMin = .75f, spawnScaleMax = 1;
	spawnSpeedMin = 8, spawnSpeedMax = 40;
	spawnAngleMin = -glm::half_pi<float>(), spawnAngleMax = -glm::half_pi<float>();
	spawnMinLife = 1.2f, spawnMaxLife = 2.6;
	shrink = true;
	fade = false;
	circleSpawn = true;
	colorShiftRate = glm::vec3(0.2f,-.05f,-1.f);
};