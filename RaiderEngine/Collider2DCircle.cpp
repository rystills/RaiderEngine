#include "stdafx.h"
#include "Collider2DCircle.hpp"
#include "Collider2DRectangle.hpp"
#include "Collider2DPolygon.hpp"
#include "Collider2DLine.hpp"
#include "terminalColors.hpp"

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