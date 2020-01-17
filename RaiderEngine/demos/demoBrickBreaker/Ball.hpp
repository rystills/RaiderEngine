#pragma once
#include "stdafx.h"
#include "GameObject2D.hpp"

class Ball : public GameObject2D {
public:
	float speed = 700;
	Ball(glm::vec2 position);

	void restart();

	void bounce(bool isVert);

	void update() override;
};