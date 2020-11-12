#include "stdafx.h"
#if (COMPILE_DEMO == DEMO_3D_CAROUSEL)
#include "FoliageGrass.hpp"
#include "GameObject.hpp"

FoliageGrass::FoliageGrass(glm::vec3 position, glm::vec3 rotationEA, glm::vec3 scale) : GameObject(position, rotationEA, scale, "foliageGrass", 1, false, false) {
	drawTwoSided = true;
}
#endif