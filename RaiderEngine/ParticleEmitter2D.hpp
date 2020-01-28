#pragma once
#include "stdafx.h"
#include "mesh.hpp"

struct Particle2D {
	glm::vec3 spriteColor;
	glm::vec2 pos;
	float scale;

	Particle2D(glm::vec3 spriteColor, glm::vec2 pos, float scale) : spriteColor(spriteColor), pos(pos), scale(scale) {}
};

class ParticleEmitter2D {
public: 
	Texture sprite;
	inline static GLuint VAO, VBO;
	inline static int numParticlesInVBO = 0;
	std::vector<Particle2D> particles;

	ParticleEmitter2D(std::string spriteName = "");

	static void initVertexObjects();

	void update();
};