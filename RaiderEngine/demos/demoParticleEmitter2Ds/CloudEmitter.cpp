#include "stdafx.h"
#if (COMPILE_DEMO == DEMO_PARTICLE_EMITTER_2DS)
#include "ParticleEmitter2D.hpp"
#include "CloudEmitter.hpp"
#include "input.hpp"
#include <limits>
#include "Collider2D.hpp"
#include "timing.hpp"

CloudEmitter::CloudEmitter(glm::vec2 pos) : ParticleEmitter2D(pos, "cloud.png") {
	spawnRMin = .8f, spawnRMax = 1, spawnGMin = .8f, spawnGMax = 1, spawnBMin = .8f, spawnBMax = 1, spawnAMin = .03f, spawnAMax = .03f;
	spawnXOffMin = -64, spawnXOffMax = 64, spawnYOffMin = -64, spawnYOffMax = 64;
	spawnScaleMin = .5f, spawnScaleMax = 1.35f;
	spawnSpeedMin = 10, spawnSpeedMax = 22;
	spawnAngleMin = 0, spawnAngleMax = glm::two_pi<float>();
	spawnMinLife = std::numeric_limits<double>::infinity(), spawnMaxLife = std::numeric_limits<double>::infinity();
	shrink = false;
	fade = false;
	circleSpawn = true;
	isburst = true;
	for (int i = 0; i < 135; ++i)
		spawnParticle();
};

void CloudEmitter::update() {
	ParticleEmitter2D::update();
	for (int i = 0; i < particles.size(); ++i) {
		// rotate each cloud particle towards the emitter's center based on its distance
		glm::vec2 posDiff = glm::normalize(particles[i].pos - pos);
		float forceDir = std::atan2(posDiff.y,posDiff.x);
		int rotDir = fmod(glm::two_pi<float>() + particleMotions[i].ang - forceDir, glm::two_pi<float>()) > glm::pi<float>() ? 1 : -1;
		particleMotions[i].ang -= rotDir * .018f * Collider2D::distance(particles[i].pos.x, particles[i].pos.y, pos.x, pos.y) * deltaTime;
	}	
}
#endif