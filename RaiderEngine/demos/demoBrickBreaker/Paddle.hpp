#pragma once
#include "GameObject2D.hpp"
#include "stdafx.h"


class Paddle : public GameObject2D {
public:
	float speed = 800;
	Paddle(glm::vec2 position);

	void restart();

	void update() override;
};
