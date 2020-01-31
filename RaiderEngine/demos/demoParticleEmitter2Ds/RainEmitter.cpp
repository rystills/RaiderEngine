#include "stdafx.h"
#include "ParticleEmitter2D.hpp"
#include "RainEmitter.hpp"
#include <limits>
#include "input.hpp"
#include "settings.hpp"

RainEmitter::RainEmitter(glm::vec2 pos) : ParticleEmitter2D(pos, "rain.png") {
	partSpawnMaxTimer = .001f;
	spawnRMin = .2f, spawnRMax = .25f, spawnGMin = .2f, spawnGMax = .25f, spawnBMin = .8f, spawnBMax = 1.f, spawnAMin = .6f, spawnAMax = .6f;
	spawnXOffMin = -80, spawnXOffMax = 80, spawnYOffMin = -2, spawnYOffMax = 2;
	spawnScaleMin = .75f, spawnScaleMax = 1;
	spawnSpeedMin = 480, spawnSpeedMax = 600;
	spawnAngleMin = glm::half_pi<float>(), spawnAngleMax = glm::half_pi<float>();
	spawnMinLife = std::numeric_limits<double>::infinity(), spawnMaxLife = std::numeric_limits<double>::infinity();
	shrink = false;
	fade = false;
	circleSpawn = false;
}

void RainEmitter::update() {
	for (int i = 0; i < particles.size(); ++i) {
		if (particles[i].pos.y > SCR_HEIGHT + sprite.height / 2)
			particleMotions[i].life = 0;
	}
	ParticleEmitter2D::update();
}