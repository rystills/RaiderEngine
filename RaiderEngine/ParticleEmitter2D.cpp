#include "stdafx.h"
#include "ParticleEmitter2D.hpp"
#include "model.hpp"
#include "settings.hpp"

ParticleEmitter2D::ParticleEmitter2D(std::string spriteName) {
	sprite = (spriteName == "" ? Model::defaultDiffuseMap : Model::loadTextureSimple(spriteName));
	/*particles.reserve(10);
	for (int i = 0; i < 10; ++i)
		particles.emplace_back(glm::vec3(1, .2f, .2f), glm::vec2(100+i*50,100+i*20),1);
*/
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
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat)));

	glVertexAttribDivisor(1, 1);
	glVertexAttribDivisor(2, 1);
	glVertexAttribDivisor(3, 1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void ParticleEmitter2D::update() {
	for (int i = 0; i < 1; ++i)
		particles.emplace_back(glm::vec3(rand() / static_cast <float> (RAND_MAX), rand() / static_cast <float> (RAND_MAX), rand() / static_cast <float> (RAND_MAX)), 
			glm::vec2(rand()/ static_cast <float> (RAND_MAX / SCR_WIDTH),rand() / static_cast <float> (RAND_MAX / SCR_HEIGHT)), 
			rand() / static_cast <float> (RAND_MAX / 8));
}