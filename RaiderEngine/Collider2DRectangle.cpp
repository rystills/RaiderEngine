#include "stdafx.h"
#include "Collider2DCircle.hpp"
#include "Collider2DRectangle.hpp"
#include "Collider2DPolygon.hpp"
#include "Collider2DLine.hpp"
#include "terminalColors.hpp"
#include "graphics.hpp"

bool Collider2DRectangle::collision(glm::vec2 myPos, float myRot, Collider2D* other, glm::vec2 otherPos, float otherRot) {
	// Rectangle <=> Rectangle collision
	if (static_cast<Collider2DRectangle*>(other)) {
		if (myRot == 0 && otherRot == 0)
			return collisionRectangleRectangle(*this, myPos, *(Collider2DRectangle*)other, otherPos);
		WARNING(puts("Rotated Rectangle/Rectangle collision type not yet implemented"))
		return false;
	}

	// Rectangle <=> Circle collision
	if (static_cast<Collider2DCircle*>(other)) {
		if (myRot == 0)
			return collisionCircleRectangle(*(Collider2DCircle*)other, otherPos, *this, myPos);
		return collisionCircleRotatedRectangle(*(Collider2DCircle*)other, otherPos, *this, myPos, myRot);
	}

	// Rectangle <=> Line collision
	if (static_cast<Collider2DLine*>(other)) {
		WARNING(puts("Rectangle/Line collision type not yet implemented"))
		return false;
	}
	
	//Rectangle <=> Polygon collision
	if (static_cast<Collider2DPolygon*>(other)) {
		WARNING(puts("Rectangle/Polygon collision type not yet implemented"))
		return false;
	}
	
	//we don't recognize the other collider's type
	WARNING(puts("Collision check attempted with unknown collider type"))
	return false;
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

void Collider2DRectangle::debugDraw(glm::vec2 pos, float rot) {
	glm::vec2 points[4] = { pos+glm::vec2(-hwidth, -hheight), pos + glm::vec2(-hwidth, hheight), pos + glm::vec2(hwidth, hheight), pos + glm::vec2(hwidth, -hheight) };
	for (int i = 0; i < 3; ++i)
		queueDrawLine(glm::vec3(points[i].x,points[i].y,0), glm::vec3(points[i+1].x, points[i + 1].y, 0), glm::vec3(1,.5f,.5f));
	queueDrawLine(glm::vec3(points[3].x, points[3].y, 0), glm::vec3(points[0].x, points[0].y, 0), glm::vec3(1, .5f, .5f));
	drawLines();
}