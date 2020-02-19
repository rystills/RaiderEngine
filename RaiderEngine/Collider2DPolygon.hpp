#pragma once
#include "stdafx.h"
#include "Collider2D.hpp"

/*
class representing a collider used for collision checking between objects
*/
class Collider2DPolygon : public Collider2D {
public:
	std::vector<glm::vec2> points;  // vector of vertex points to which rotations are directly applied

	/*
	ColliderPolygon constructor: create a new polygon collider with the given vertices
	@param inPoints: the list of relative point positions to this collider's center
	*/
	Collider2DPolygon(std::vector<glm::vec2> inPoints, int collisionLayer = 0);

	/*
	check whether two colliders are overlapping (true) or not (false)
	@param myPos: the position of this collider
	@param other: the collider with which to check for a collision
	@param otherPos: the position of the other collider
	@param otherRot: the rotation of the other collider
	@returns: whether the two colliders are overlapping (true) or not (false)
	*/
	bool collision(glm::vec2 myPos, float myRot, Collider2D* other, glm::vec2 otherPos, float otherRot = 0) override;

	void getRotatedPoints(glm::vec2 pts[], glm::vec2 pos, float rot);

	void debugDraw(glm::vec2 pos, float rot) override;
};