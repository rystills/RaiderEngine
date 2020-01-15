#pragma once
#include "stdafx.h"
#include "Collider2D.hpp"

/*
class representing a collider used for collision checking between objects
*/
class Collider2DCircle : public Collider2D {
public:
	float radius;

	/*
	ColliderCircle constructor: create a new circular collider with the given radius
	@param radius: the radius of the circle
	@param height: the height of the rectangle
	*/
	Collider2DCircle(float radius);

	/*
	check whether two colliders are overlapping (true) or not (false)
	@param myPos">the position of this collider
	@param other">the collider with which to check for a collision
	@param otherPos">the position of the other collider
	@param otherRot">the rotation of the other collider
	@returns whether the two colliders are overlapping (true) or not (false)
	*/
	bool collision(glm::vec2 myPos, float myRot, Collider2D* other, glm::vec2 otherPos, float otherRot = 0) override;

	void debugDraw(glm::vec2 pos, float rot) override;
};