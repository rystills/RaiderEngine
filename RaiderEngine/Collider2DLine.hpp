#pragma once
#include "stdafx.h"
#include "Collider2D.hpp"

/*
class representing a collider used for collision checking between objects
*/
class Collider2DLine : public Collider2D {
public:
	float sx, sy, ex, ey;

	/*
	ColliderLine constructor: create a new line collider with the given start and end coordinates
	@param sx: the starting x coordinate of the line
	@param sy: the starting y coordinate of the line
	@param ex: the ending x coordinate of the line
	@param ey: the ending y coordinate of the line
	*/
	Collider2DLine(float sx, float sy, float ex, float ey) : sx(sx), sy(sy), ex(ex), ey(ey) { }

	/*
	check whether two colliders are overlapping (true) or not (false)
	@param myPos: the position of this collider
	@param other: the collider with which to check for a collision
	@param otherPos: the position of the other collider
	@param otherRot: the rotation of the other collider
	@returns: whether the two colliders are overlapping (true) or not (false)
	*/
	bool collision(glm::vec2 myPos, float myRot, Collider2D* other, glm::vec2 otherPos, float otherRot = 0) override;

	void debugDraw(glm::vec2 pos, float rot) override;
};