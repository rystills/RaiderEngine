#include "stdafx.h"
#if (COMPILE_DEMO == DEMO_PARTICLE_EMITTER_2DS)
#include "ParticleEmitter2D.hpp"
#include "FireEmitter.hpp"
#include "input.hpp"

FireEmitter::FireEmitter(glm::vec2 pos) : ParticleEmitter2D(pos, "fire.png") {
	partSpawnMaxTimer = .001f;
	spawnRMin = .7f, spawnRMax = 1.f, spawnGMin = .1f, spawnGMax = .4f, spawnBMin = .05f, spawnBMax = .15f, spawnAMin = .4f, spawnAMax = .4f;
	spawnXOffMin = -40, spawnXOffMax = 40, spawnYOffMin = -10, spawnYOffMax = 10;
	spawnScaleMin = .75f, spawnScaleMax = 1;
	spawnSpeedMin = 16, spawnSpeedMax = 80;
	spawnAngleMin = -glm::half_pi<float>(), spawnAngleMax = -glm::half_pi<float>();
	spawnMinLife = 1.7f, spawnMaxLife = 3.7f;
	shrink = true;
	fade = false;
	circleSpawn = true;
	colorShiftRate = glm::vec3(0.2f,-.05f,-1.f);
};
#endif