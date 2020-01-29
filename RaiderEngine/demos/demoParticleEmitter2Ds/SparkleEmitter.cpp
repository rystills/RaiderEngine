#include "stdafx.h"
#include "ParticleEmitter2D.hpp"
#include "SparkleEmitter.hpp"
#include "input.hpp"

SparkleEmitter::SparkleEmitter(glm::vec2 pos) : ParticleEmitter2D(pos, "star.png") {
	partSpawnMaxTimer = .004f;
	spawnRMin = 0, spawnRMax = 1, spawnGMin = 0, spawnGMax = 1, spawnBMin = 0, spawnBMax = 1;
	spawnXOffMin = -10, spawnXOffMax = 10, spawnYOffMin = -10, spawnYOffMax = 10;
	spawnScaleMin = .5f, spawnScaleMax = 1.75f;
	spawnSpeedMin = 40, spawnSpeedMax = 175;
	spawnAngleMin = 0, spawnAngleMax = glm::two_pi<float>();
	spawnMinLife = .5f, spawnMaxLife = .5f;
	shrink = true;
	fade = true;
	circleSpawn = true;
}

void SparkleEmitter::update() {
	if (!firstMouse) {
		pos.x = lastX;
		pos.y = lastY;
	}
	ParticleEmitter2D::update();
}