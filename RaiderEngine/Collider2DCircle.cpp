#include "stdafx.h"
#include "Collider2DCircle.hpp"
#include "Collider2DRectangle.hpp"
#include "Collider2DPolygon.hpp"
#include "Collider2DLine.hpp"
#include "terminalColors.hpp"
#include "graphics.hpp"

Collider2DCircle::Collider2DCircle(float radius, int collisionLayer) : radius(radius), Collider2D(circle, collisionLayer) {
	boundingRadius = radius;
}

bool Collider2DCircle::collision(glm::vec2 myPos, float myRot, Collider2D* other, glm::vec2 otherPos, float otherRot) {
	if (!boundingRadiusCheck(*this, myPos, *other, otherPos))
		return false;
	switch (other->type) {
	case rectangle:
		if (otherRot == 0)
			return collisionCircleRectangle(*this, myPos, *(Collider2DRectangle*)other, otherPos);
		return collisionCircleRotatedRectangle(*this, myPos, *(Collider2DRectangle*)other, otherPos, otherRot);
	case circle:
		return collisionCircleCircle(*this, myPos, *(Collider2DCircle*)(other), otherPos);
	case polygon:
		if (otherRot == 0)
			return collisionCirclePolygon(*this, myPos, *(Collider2DPolygon*)other, otherPos);
		return collisionCircleRotatedPolygon(*this, myPos, *(Collider2DPolygon*)other, otherPos, otherRot);
	case line:
		if (otherRot == 0)
			return collisionCircleLine(*this, myPos, *(Collider2DLine*)other, otherPos);
		return collisionCircleRotatedLine(*this, myPos, *(Collider2DLine*)other, otherPos, otherRot);
	default:
		WARNINGCOLOR(puts("Collision check attempted with unknown collider type"));
		return false;
	}
}

void Collider2DCircle::debugDraw(glm::vec2 pos, float rot) {
	const int numCirclePoints = 18;
	glm::vec2 points[numCirclePoints];
	for (int i = 0; i < numCirclePoints; ++i) {
		float ang = glm::two_pi<float>() * (i / (float)numCirclePoints);
		points[i] = glm::vec2(pos.x + cos(ang)*radius, pos.y + sin(ang)*radius);
	}
	for (int i = 0; i < numCirclePoints-1; ++i)
		queueDrawLine(glm::vec3(points[i].x, points[i].y, 0), glm::vec3(points[i + 1].x, points[i + 1].y, 0), stateColors[collisionLayer % stateColors->length()]);
	queueDrawLine(glm::vec3(points[numCirclePoints-1].x, points[numCirclePoints - 1].y, 0), glm::vec3(points[0].x, points[0].y, 0), stateColors[collisionLayer % stateColors->length()]);
}