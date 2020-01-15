#include "stdafx.h"
#include "Collider2DCircle.hpp"
#include "Collider2DRectangle.hpp"
#include "Collider2DPolygon.hpp"
#include "Collider2DLine.hpp"
#include "terminalColors.hpp"
#include "graphics.hpp"

Collider2DLine::Collider2DLine(float sx, float sy, float ex, float ey) : sx(sx), sy(sy), ex(ex), ey(ey) {
	glm::vec2 pts[2] = { glm::vec2(sx,sy),glm::vec2(ex,ey) };
	calculateBoundingRadius(pts,2);
}

bool Collider2DLine::collision(glm::vec2 myPos, float myRot, Collider2D* other, glm::vec2 otherPos, float otherRot) {
	if (!boundingRadiusCheck(*this, myPos, *other, otherPos))
		return false;
	// Line <=> Rectangle collision
	if (dynamic_cast<Collider2DRectangle*>(other)) {
		return collisionLineRectangle(*this, myPos, myRot, *(Collider2DRectangle*)other, otherPos, otherRot);
	}

	// Line <=> Circle collision
	if (dynamic_cast<Collider2DCircle*>(other)) {
		if (myRot == 0)
			return collisionCircleLine(*(Collider2DCircle*)other, otherPos, *this, myPos);
		return collisionCircleRotatedLine(*(Collider2DCircle*)other, otherPos, *this, myPos, myRot);
	}		

	// Line <=> Polygon collision
	if (dynamic_cast<Collider2DPolygon*>(other)) {
		return collisionLinePolygon(*this, myPos, myRot, *(Collider2DPolygon*)other, otherPos, otherRot);
	}

	//Line <=> Line collision
	if (dynamic_cast<Collider2DLine*>(other)) {
		return collisionLineLine(*this, myPos, myRot, *(Collider2DLine*)other, otherPos, otherRot);
	}
	// we don't recognize the other collider's type
	WARNING(puts("Collision check attempted with unknown collider type"))
	return false;
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
	drawLines();
}