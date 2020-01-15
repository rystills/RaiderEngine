#include "stdafx.h"
#include "Collider2DCircle.hpp"
#include "Collider2DRectangle.hpp"
#include "Collider2DPolygon.hpp"
#include "Collider2DLine.hpp"
#include "terminalColors.hpp"
#include "graphics.hpp"

bool Collider2DLine::collision(glm::vec2 myPos, float myRot, Collider2D* other, glm::vec2 otherPos, float otherRot) {
	return false;
}

void Collider2DLine::debugDraw(glm::vec2 pos, float rot) {
	glm::vec2 pt1(sx,sy), pt2(ex,ey);
	RotatePoint(pt1, glm::vec2(0, 0), rot);
	RotatePoint(pt2, glm::vec2(0, 0), rot);
	queueDrawLine(glm::vec3(pos.x + pt1.x, pos.y + pt1.y,0), glm::vec3(pos.x + pt2.x, pos.y + pt2.y,0), glm::vec3(1, .5f, .5f));
	drawLines();
}