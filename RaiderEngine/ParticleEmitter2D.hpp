#pragma once
#include "stdafx.h"
#include "mesh.hpp"

struct Particle2D {
	glm::vec3 spriteColor;
	glm::vec2 pos;
	float scale;

	Particle2D(glm::vec3 spriteColor, glm::vec2 pos, float scale) : spriteColor(spriteColor), pos(pos), scale(scale) {}
};

struct ParticleMotionData2D {
	float speed, ang, life, maxScale;
	ParticleMotionData2D(float speed, float ang, float life, float maxScale) : speed(speed), ang(ang), life(life), maxScale(maxScale) {}
};

class ParticleEmitter2D {
public: 
	Texture sprite;
	inline static GLuint VAO, VBO;
	inline static int numParticlesInVBO = 0;
	float partSpawnTimer = 0.f, partSpawnMaxTimer = .001f;
	std::vector<Particle2D> particles;
	std::vector<ParticleMotionData2D> particleMotions;
	glm::vec2 pos;

	ParticleEmitter2D(glm::vec2 pos, std::string spriteName = "");

	static void initVertexObjects();

	void update();
};