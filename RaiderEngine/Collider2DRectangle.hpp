#pragma once
#include "stdafx.h"
#include "Collider2D.hpp"

/*
class representing a collider used for collision checking between objects
*/
class Collider2DRectangle : public Collider2D {
public:
	float hwidth, hheight;

	/*
	ColliderRectangle constructor: create a new rectangular collider with the given width and height
	@param hwidth: the half width of the rectangle
	@param hheight: the half height of the rectangle
	*/
	Collider2DRectangle(float hwidth, float hheight) : hwidth(hwidth), hheight(hheight) {}

	/*
	check whether two colliders are overlapping (true) or not (false)
	@param myPos: the position of this collider
	@param other: the collider with which to check for a collision
	@param otherPos: the position of the other collider
	@param otherRot: the rotation of the other collider
	@returns: whether the two colliders are overlapping (true) or not (false)
	*/
	bool collision(glm::vec2 myPos, float myRot, Collider2D* other, glm::vec2 otherPos, float otherRot = 0) override;

	/*
	calculate the 4 ordered corner points of the specified rectangle collider rotated by the specified angle (in radians)
	@param rect: the rectangle collider to rotate
	@param rectPos: the position of the rectangle
	@param ang: the angle (in radians) by which to rotate the collider
	*/
	void rotateCornerPoints(glm::vec2 pts[], glm::vec2 pos, float ang);

	void debugDraw(glm::vec2 pos, float rot) override;
};