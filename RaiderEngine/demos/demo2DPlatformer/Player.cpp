#include "stdafx.h"
#if (COMPILE_DEMO == DEMO_2D_PLATFORMER)
#include "player.hpp"
#include "input.hpp"
#include "timing.hpp"
#include "settings.hpp"
#include "Collider2DRectangle.hpp"

Player::Player(glm::vec2 position) : GameObject2D(position, 0, glm::vec2(1), glm::vec3(1), "player.png") {
	collider = colliders.contains("player") ? colliders["player"].get() : addCollider2D("player", new Collider2DRectangle(4,4));
}

void Player::update() {
}
#endif