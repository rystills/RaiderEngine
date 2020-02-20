#include "stdafx.h"
#include "Collider2DCircle.hpp"
#include "Collider2DRectangle.hpp"
#include "Collider2DPolygon.hpp"
#include "Collider2DLine.hpp"
#include "terminalColors.hpp"
#include "graphics.hpp"

Collider2DLine::Collider2DLine(float sx, float sy, float ex, float ey, int collisionLayer) : sx(sx), sy(sy), ex(ex), ey(ey), Collider2D(line, collisionLayer) {
	glm::vec2 pts[2] = { glm::vec2(sx,sy),glm::vec2(ex,ey) };
	calculateBoundingRadius(pts,2);
}

bool Collider2DLine::collision(glm::vec2 myPos, float myRot, Collider2D* other, glm::vec2 otherPos, float otherRot) {
	if (!boundingRadiusCheck(*this, myPos, *other, otherPos))
		return false;
	switch (other->type) {
	case rectangle:
		return collisionLineRectangle(*this, myPos, myRot, *(Collider2DRectangle*)other, otherPos, otherRot);
	case circle:
		if (myRot == 0)
			return collisionCircleLine(*(Collider2DCircle*)other, otherPos, *this, myPos);
		return collisionCircleRotatedLine(*(Collider2DCircle*)other, otherPos, *this, myPos, myRot);
	case polygon:
		return collisionLinePolygon(*this, myPos, myRot, *(Collider2DPolygon*)other, otherPos, otherRot);
	case line:
		return collisionLineLine(*this, myPos, myRot, *(Collider2DLine*)other, otherPos, otherRot);
	default:
		WARNINGCOLOR(puts("Collision check attempted with unknown collider type"))
		return false;
	}
}

void Collider2DLine::getRotatedPoints(glm::vec2 pts[], glm::vec2 pos, float rot) {
	pts[0] = glm::vec2(sx, sy);
	pts[1] = glm::vec2(ex, ey);
	RotatePoint(pts[0], glm::vec2(0, 0), rot);
	RotatePoint(pts[1], glm::vec2(0, 0), rot);
	pts[0] += pos;
	pts[1] += pos;
}

void Collider2DLine::debugDraw(glm::vec2 pos, float rot) {
	glm::vec2 points[2];
	getRotatedPoints(points, pos, rot);
	queueDrawLine(glm::vec3(points[0].x, points[0].y,0), glm::vec3(points[1].x, points[1].y,0), glm::vec3(1, .5f, .5f));
}