#pragma once
#include "stdafx.h"
#include "ParticleEmitter2D.hpp"

class RainEmitter : public ParticleEmitter2D {
public:
	RainEmitter(glm::vec2 pos);

	void update() override;
};