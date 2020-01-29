#include "stdafx.h"
#include "ParticleEmitter2D.hpp"
#include "model.hpp"
#include "settings.hpp"
#include "timing.hpp"

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

	// configure VBO for vec3 spriteColor (attrib 1), vec2 pos (attrib 2), float scale (attrib 3), 
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat)));

	glVertexAttribDivisor(1, 1);
	glVertexAttribDivisor(2, 1);
	glVertexAttribDivisor(3, 1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void ParticleEmitter2D::update() {
	// tick old particles
	for (int i = 0; i < particles.size(); ++i) {
		if ((particleMotions[i].life -= .7f*deltaTime) <= 0) {
			particles.erase(particles.begin()+i);
			particleMotions.erase(particleMotions.begin() + i);
			--i;
			continue;
		}
		particles[i].pos.x += cos(particleMotions[i].ang) * particleMotions[i].speed * deltaTime;
		particles[i].pos.y += sin(particleMotions[i].ang) * particleMotions[i].speed * deltaTime;
		particles[i].scale = particleMotions[i].maxScale * particleMotions[i].life;
	}
	// create new particles
	partSpawnTimer -= deltaTime;
	while (partSpawnTimer <= 0) {
		particles.emplace_back(glm::vec3(rand() / static_cast <float> (RAND_MAX), rand() / static_cast <float> (RAND_MAX), rand() / static_cast <float> (RAND_MAX)),
			pos + glm::vec2(rand() / static_cast <float> (RAND_MAX / 10), rand() / static_cast <float> (RAND_MAX / 10)),
			rand() / static_cast <float> (RAND_MAX / 3));
		particleMotions.emplace_back(100 + rand() / static_cast <float> (RAND_MAX / 400), rand() / static_cast <float> (RAND_MAX / glm::two_pi<float>()), 1, particles[particles.size()-1].scale);
		partSpawnTimer += partSpawnMaxTimer;
	}
}