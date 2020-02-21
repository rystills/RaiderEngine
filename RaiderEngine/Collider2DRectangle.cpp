#include "stdafx.h"
#include "Collider2DCircle.hpp"
#include "Collider2DRectangle.hpp"
#include "Collider2DPolygon.hpp"
#include "Collider2DLine.hpp"
#include "terminalColors.hpp"
#include "graphics.hpp"

Collider2DRectangle::Collider2DRectangle(float hwidth, float hheight, int collisionLayer) : hwidth(hwidth), hheight(hheight), Collider2D(rectangle, collisionLayer) {
	glm::vec2 pts[4];
	getCornerPoints(pts, glm::vec2(0));
	calculateBoundingRadius(pts, 4);
}

bool Collider2DRectangle::collision(glm::vec2 myPos, float myRot, Collider2D* other, glm::vec2 otherPos, float otherRot) {
	if (!boundingRadiusCheck(*this, myPos, *other, otherPos))
		return false;
	switch (other->type) {
	case rectangle:
		if (myRot == 0 && otherRot == 0)
			return collisionRectangleRectangle(*this, myPos, *(Collider2DRectangle*)other, otherPos);
		return collisionRectangleRotatedRectangle(*this, myPos, myRot, *(Collider2DRectangle*)other, otherPos, otherRot);
	case circle:
		if (myRot == 0)
			return collisionCircleRectangle(*(Collider2DCircle*)other, otherPos, *this, myPos);
		return collisionCircleRotatedRectangle(*(Collider2DCircle*)other, otherPos, *this, myPos, myRot);
	case line:
		return collisionLineRectangle(*(Collider2DLine*)other, otherPos, otherRot, *this, myPos, myRot);
	case polygon:
		return collisionRectanglePolygon(*this, myPos, myRot, *(Collider2DPolygon*)other, otherPos, otherRot);
	default:
		WARNINGCOLOR(puts("Collision check attempted with unknown collider type"))
		return false;
	}
}

void Collider2DRectangle::rotateCornerPoints(glm::vec2 pts[], glm::vec2 pos, float ang) {
	float cosA = cos(ang);
	float sinA = sin(ang);
	for (int sign1 = -1; sign1 < 2; sign1 += 2) {
		for (int sign2 = -1; sign2 < 2; sign2 += 2) {
			pts[(sign1 + 1) + ((sign2 + 1) / 2)].x = pos.x + sign1 * hwidth * cosA - sign2 * hheight * sinA;
			pts[(sign1 + 1) + ((sign2 + 1) / 2)].y = pos.y + sign1 * hwidth * sinA + sign2 * hheight * cosA;
		}
	}
	//swap indices 2 and 3 so that the points are ordered in a way that properly defines the rectangle's border
	std::swap(pts[2], pts[3]);
}

void Collider2DRectangle::getCornerPoints(glm::vec2 pts[], glm::vec2 pos) {
	pts[0] = pos + glm::vec2(-hwidth, -hheight);
	pts[1] = pos + glm::vec2(-hwidth, hheight);
	pts[2] = pos + glm::vec2(hwidth, hheight);
	pts[3] = pos + glm::vec2(hwidth, -hheight);
}

void Collider2DRectangle::getRotatedCornerPoints(glm::vec2 pts[], glm::vec2 pos, float rot) {
	rotateCornerPoints(pts,pos,rot);
}

void Collider2DRectangle::debugDraw(glm::vec2 pos, float rot) {
	glm::vec2 points[4];
	getRotatedCornerPoints(points,pos,rot);
	for (int i = 0; i < 3; ++i)
		queueDrawLine(glm::vec3(points[i].x,points[i].y,0), glm::vec3(points[i+1].x, points[i + 1].y, 0), stateColors[collisionLayer%stateColors->length()]);
	queueDrawLine(glm::vec3(points[3].x, points[3].y, 0), glm::vec3(points[0].x, points[0].y, 0), stateColors[collisionLayer%stateColors->length()]);
}