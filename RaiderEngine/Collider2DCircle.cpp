#include "stdafx.h"
#include "Collider2DCircle.hpp"
#include "Collider2DRectangle.hpp"
#include "Collider2DPolygon.hpp"
#include "Collider2DLine.hpp"
#include "terminalColors.hpp"
#include "graphics.hpp"

bool Collider2DCircle::collision(glm::vec2 myPos, float myRot, Collider2D* other, glm::vec2 otherPos, float otherRot) {
	//Circle <=> Rectangle collision
	/*if (dynamic_cast<Collider2DRectangle*>(other)) {
		if (otherRot == 0)
			return collisionCircleRectangle(*this, myPos, *dynamic_cast<Collider2DRectangle*>(other), otherPos);
		return collisionCircleRotatedRectangle(*this, myPos, *dynamic_cast<Collider2DRectangle*>(other), otherPos, otherRot);
	}

	//Circle <=> Circle collision
	if (dynamic_cast<Collider2DCircle*>(other))
		return collisionCircleCircle(*this, myPos, *dynamic_cast<Collider2DCircle*>(other), otherPos);

	//Circle <=> Polygon collision
	if (dynamic_cast<Collider2DPolygon*>(other)) {
		if (otherRot == 0)
			return collisionCirclePolygon(*this, myPos, *dynamic_cast<Collider2DPolygon*>(other), otherPos);
		return collisionCircleRotatedPolygon(*this, myPos, *dynamic_cast<Collider2DPolygon*>(other), otherPos, otherRot);
	}

	//Circle <=> Line collision
	if (dynamic_cast<Collider2DLine*>(other))
		return collisionCircleLine(*this, myPos, *dynamic_cast<Collider2DLine*>(other), otherPos);

	//we don't recognize the other collider's type
	return false;
	WARNING(puts("Collision check attempted with unknown collider type"))*/
	return false;
}

void Collider2DCircle::debugDraw(glm::vec2 pos, float rot) {
	const int numCirclePoints = 18;
	glm::vec2 points[numCirclePoints];
	for (int i = 0; i < numCirclePoints; ++i) {
		float ang = glm::two_pi<float>() * (i / (float)numCirclePoints);
		points[i] = glm::vec2(pos.x + cos(ang)*radius, pos.y + sin(ang)*radius);
	}
	for (int i = 0; i < numCirclePoints-1; ++i)
		queueDrawLine(glm::vec3(points[i].x, points[i].y, 0), glm::vec3(points[i + 1].x, points[i + 1].y, 0), glm::vec3(1, .5f, .5f));
	queueDrawLine(glm::vec3(points[numCirclePoints-1].x, points[numCirclePoints - 1].y, 0), glm::vec3(points[0].x, points[0].y, 0), glm::vec3(1, .5f, .5f));
	drawLines();
}