#include "stdafx.h"
#include "Brick.hpp"
#include "settings.hpp"
#include "Collider2DRectangle.hpp"

Brick::Brick(glm::vec2 position, glm::vec3 color) : GameObject2D(position, 0, glm::vec2(1), color, "brick.png") {
	collider = colliders.contains("brick") ? colliders["brick"].get() : addCollider2D("brick", new Collider2DRectangle(32,16));
}