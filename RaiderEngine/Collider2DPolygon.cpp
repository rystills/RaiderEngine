#include "stdafx.h"
#include "Collider2DCircle.hpp"
#include "Collider2DRectangle.hpp"
#include "Collider2DPolygon.hpp"
#include "Collider2DLine.hpp"
#include "terminalColors.hpp"
#include "graphics.hpp"

Collider2DPolygon::Collider2DPolygon(std::vector<glm::vec2> inPoints) {
	//create a deep copy of the input vector2 array so we can modify points locally
	for (int i = 0; i < inPoints.size(); ++i)
		points.push_back(glm::vec2(inPoints[i].x, inPoints[i].y));
}

bool Collider2DPolygon::collision(glm::vec2 myPos, float myRot, Collider2D* other, glm::vec2 otherPos, float otherRot) {
	return false;
}

void Collider2DPolygon::debugDraw(glm::vec2 pos, float rot) {
	glm::vec2 pt1(points[points.size()-1].x, points[points.size() - 1].y);
	RotatePoint(pt1, glm::vec2(0, 0), rot);
	glm::vec2 pt2;
	for (int i = 0; i < points.size(); ++i) {
		pt2 = glm::vec2(points[i].x, points[i].y);
		RotatePoint(pt2, glm::vec2(0, 0), rot);
		queueDrawLine(glm::vec3(pos.x + pt1.x, pos.y + pt1.y, 0), glm::vec3(pos.x + pt2.x, pos.y + pt2.y, 0), glm::vec3(1, .5f, .5f));
		pt1 = pt2;
	}
	drawLines();
}