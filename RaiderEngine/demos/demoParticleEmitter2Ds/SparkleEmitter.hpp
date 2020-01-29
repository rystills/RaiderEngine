#pragma once
#include "stdafx.h"
#include "ParticleEmitter2D.hpp"

class SparkleEmitter : public ParticleEmitter2D {
public:
	SparkleEmitter(glm::vec2 pos);

	void update() override;
};