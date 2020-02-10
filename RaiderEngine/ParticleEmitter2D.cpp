#include "stdafx.h"
#include "ParticleEmitter2D.hpp"
#include "model.hpp"
#include "settings.hpp"
#include "timing.hpp"
#include "input.hpp"
// TODO: define this as a proper function in settings, and overload with ints as well
#define randRange(LO, HI) HI==LO ? LO : LO + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(HI- LO)))

ParticleEmitter2D::ParticleEmitter2D(glm::vec2 pos, std::string spriteName) : pos(pos) {
	sprite = (spriteName == "" ? Model::defaultDiffuseMap : Model::loadTextureSimple(spriteName));
}

void ParticleEmitter2D::initVertexObjects() {
	// Configure VAO and temporary VBO
	GLuint tempVBO;
	GLfloat vertices[] = {
		// Pos      // Tex
		0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
	};

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &tempVBO);

	glBindBuffer(GL_ARRAY_BUFFER, tempVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindVertexArray(VAO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);

	// configure VBO for vec4 spriteColor (attrib 1), vec2 pos (attrib 2), float scale (attrib 3), 
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (GLvoid*)(4 * sizeof(GLfloat)));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));

	glVertexAttribDivisor(1, 1);
	glVertexAttribDivisor(2, 1);
	glVertexAttribDivisor(3, 1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void ParticleEmitter2D::spawnParticle() {
	float sx, sy;
	if (circleSpawn) {
		float positionAngle = randRange(0, glm::two_pi<float>());
		float positionDist = randRange(spawnXOffMin, spawnXOffMax);
		sx = cos(positionAngle) * positionDist;
		sy = sin(positionAngle) * positionDist;
	}
	else {
		sx = randRange(spawnXOffMin, spawnXOffMax);
		sy = randRange(spawnYOffMin, spawnYOffMax);
	}

	if (recycleParticles && !recycledParticleInds.empty()) {
		particles[recycledParticleInds.back()] = { glm::vec4(randRange(spawnRMin, spawnRMax), randRange(spawnGMin, spawnGMax), randRange(spawnBMin, spawnBMax), randRange(spawnAMin, spawnAMax)), pos + glm::vec2(sx, sy), randRange(spawnScaleMin, spawnScaleMax) };
		particleMotions[recycledParticleInds.back()] = { randRange(spawnSpeedMin, spawnSpeedMax), randRange(spawnAngleMin, spawnAngleMax), randRange(spawnMinLife, spawnMaxLife), particles[recycledParticleInds.back()].scale, particles[recycledParticleInds.back()].spriteColor.a };
		recycledParticleInds.pop_back();
		return;
	}
	particles.emplace_back(glm::vec4(randRange(spawnRMin, spawnRMax), randRange(spawnGMin, spawnGMax), randRange(spawnBMin, spawnBMax), randRange(spawnAMin, spawnAMax)), pos + glm::vec2(sx, sy), randRange(spawnScaleMin, spawnScaleMax));
	particleMotions.emplace_back(randRange(spawnSpeedMin, spawnSpeedMax), randRange(spawnAngleMin, spawnAngleMax), randRange(spawnMinLife, spawnMaxLife), particles.back().scale, particles.back().spriteColor.a);
}

bool ParticleEmitter2D::killParticle(int partInd) {
	if (recycleParticles) {
		particleMotions[partInd].life = 0;
		particles[partInd].scale = 0;
		particles[partInd].spriteColor.a = 0;
		recycledParticleInds.push_back(partInd);
		return false;
	}
	particles.erase(particles.begin() + partInd);
	particleMotions.erase(particleMotions.begin() + partInd);
	return true;
}

void ParticleEmitter2D::update() {
	// tick old particles
	for (unsigned int i = 0; i < particles.size(); ++i) {
		// if the particle is already dead, that can only mean it's awaiting recycling
		if (particleMotions[i].life <= 0)
			continue;
		// the particle died just now
		if ((particleMotions[i].life -= .7f*deltaTime) <= 0) {
			i-= killParticle(i);
			continue;
		}
		particles[i].pos.x += cos(particleMotions[i].ang) * particleMotions[i].speed * deltaTime;
		particles[i].pos.y += sin(particleMotions[i].ang) * particleMotions[i].speed * deltaTime;
		if (shrink)
			particles[i].scale = particleMotions[i].maxScale * (particleMotions[i].life / particleMotions[i].maxLife);
		if (fade)
			particles[i].spriteColor.a = particleMotions[i].maxAlpha * (particleMotions[i].life / particleMotions[i].maxLife);
		if (colorShift) {
			particles[i].spriteColor.r += colorShiftRate.r * deltaTime;
			particles[i].spriteColor.g += colorShiftRate.g * deltaTime;
			particles[i].spriteColor.b += colorShiftRate.b * deltaTime;
		}
	}
	// create new particles
	if (!isburst) {
		partSpawnTimer -= deltaTime;
		while (partSpawnTimer <= 0) {
			spawnParticle();
			partSpawnTimer += partSpawnMaxTimer;
		}
	}
}