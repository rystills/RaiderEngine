#include "stdafx.h"
#include "Collider2DCircle.hpp"
#include "Collider2DRectangle.hpp"
#include "Collider2DPolygon.hpp"
#include "Collider2DLine.hpp"
#include "terminalColors.hpp"

Collider2DPolygon::Collider2DPolygon(std::vector<glm::vec2> inPoints) {
	//create a deep copy of the input vector2 array so we can modify points locally
	for (int i = 0; i < inPoints.size(); ++i)
		points.push_back(glm::vec2(inPoints[i].x, inPoints[i].y));
}

bool Collider2DPolygon::collision(glm::vec2 myPos, float myRot, Collider2D* other, glm::vec2 otherPos, float otherRot) {
	return false;
}