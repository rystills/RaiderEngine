#pragma once
#include "stdafx.h"
#include "ParticleEmitter2D.hpp"

class CloudEmitter : public ParticleEmitter2D {
public:
	CloudEmitter(glm::vec2 pos);
	void update() override;
};