#pragma once
#include "stdafx.h"
#include "GameObject2D.hpp"

class Player : public GameObject2D {
public:
	Player(glm::vec2 position);

	void update() override;
};
