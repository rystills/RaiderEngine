#include "stdafx.h"
#if (COMPILE_DEMO == DEMO_BRICK_BREAKER)
#include "Ball.hpp"
#include "GameManager.hpp"
#include "settings.hpp"
#include "timing.hpp"
#include "Collider2DCircle.hpp"

void Ball::restart() {
	setCenter(TARGET_WIDTH / 2.f, TARGET_HEIGHT / 2.f + 200);
	setRot(-glm::quarter_pi<float>());
}

Ball::Ball(glm::vec2 position) : GameObject2D(position, 0, glm::vec2(1), glm::vec3(1), "ball.png") {
	collider = colliders.contains("ball") ? colliders["ball"].get() : addCollider2D("ball", new Collider2DCircle(12));
}

void Ball::bounce(bool isVert) {
	// bounce by flipping either the x or y component of the rotation vector
	setRot(std::atan2(sin(rotation) * (isVert ? -1 : 1), cos(rotation) * (isVert ? 1 : -1)));
}

void Ball::update() {
	if (paused)
		return;
	// move forward
	translate(cos(rotation) * speed * deltaTime * levelMultiplier, sin(rotation) * speed * deltaTime * levelMultiplier);

	// bounce off of screen borders, restarting the game on contact with the bottom of the screen
	if (center.y > TARGET_HEIGHT) {
		paused = true;
		return;
	}
	if (center.y < 0) {
		setCenter(center.x, 0);
		bounce(1);
	}

	if (center.x > TARGET_WIDTH) {
		setCenter(static_cast<float>(TARGET_WIDTH), center.y);
		bounce(0);
	}
	else if (center.x < 0) {
		setCenter(0, center.y);
		bounce(0);
	}

	// bounce off of bricks, breaking them in the process
	for (unsigned int i = 0; i < gameObject2Ds["brick"].size(); ++i) {
		if (collidesWith(&*gameObject2Ds["brick"][i])) {
			GameObject2D o = *gameObject2Ds["brick"][i];
			// if we are moving away from the brick on one axis, the correct bounce must be on the other axis
			if (center.x > o.center.x&& cos(rotation) > 0 || center.x < o.center.x && cos(rotation) < 0)
				bounce(1);
			else if (center.y > o.center.y&& sin(rotation) > 0 || center.y < o.center.y && sin(rotation) < 0)
				bounce(0);
			else {
				// either axis could be a valid bounce, so choose the axis on which we are penetrating the brick the least
				float xPen = std::fmin(std::abs(position.x + sprite.width - o.position.x), std::abs(o.position.x + o.sprite.width - position.x));
				float yPen = std::fmin(std::abs(position.y + sprite.height - o.position.y), std::abs(o.position.y + o.sprite.height - position.y));
				bounce(xPen > yPen);
			}
			// destroy the brick and update our score
			removeGameObject2D("brick", i);
			score += 100;
			textObjects[1]->text = "Score: " + std::to_string(score);
			break;
		}
	}

	// bounce off of the paddle, picking a new direction based off of our horizontal distance from the paddle's center
	if (collidesWith(paddle)) {
		setPos(position.x,paddle->position.y - sprite.height);
		float xOff = center.x - paddle->center.x;
		setRot(-glm::half_pi<float>() + (3 * glm::pi<float>() / 8) * (xOff / (sprite.width / 2 + paddle->sprite.width / 2)));
	}
}
#endif