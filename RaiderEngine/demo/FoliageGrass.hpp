#pragma once
#include "stdafx.h"
#include "GameObject.hpp"

class FoliageGrass : public GameObject {
public:
	FoliageGrass(glm::vec3 position, glm::vec3 rotationEA, glm::vec3 scale) : GameObject(position, rotationEA, scale, "foliageGrass", 1, false, true, false) {}
};