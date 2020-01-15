#include "stdafx.h"
#include "Collider2D.hpp"
#include "Collider2DCircle.hpp"
#include "Collider2DRectangle.hpp"
#include "Collider2DPolygon.hpp"
#include "Collider2DLine.hpp"
#include "settings.hpp"

bool Collider2D::collision(glm::vec2 myPos, float myRot, Collider2D* other, glm::vec2 otherPos, float otherRot) {
	return false;
}

bool Collider2D::collisionRectangleRectangle(Collider2DRectangle a, glm::vec2 aPos, Collider2DRectangle b, glm::vec2 bPos) {
	return !(
		(aPos.x - a.hwidth > bPos.x + b.hwidth || bPos.x - b.hwidth > aPos.x + a.hwidth) ||
		(aPos.y - a.hheight > bPos.y + b.hheight || bPos.y - b.hheight > aPos.y + a.hheight)
		);
}

bool Collider2D::collisionCircleRectangle(Collider2DCircle a, glm::vec2 aPos, Collider2DRectangle b, glm::vec2 bPos) {
	float deltaX = aPos.x - std::max(bPos.x - b.hwidth, std::min(aPos.x, bPos.x + b.hwidth));
	float deltaY = aPos.y - std::max(bPos.y - b.hheight, std::min(aPos.y, bPos.y + b.hheight));
	return (deltaX * deltaX + deltaY * deltaY) < (a.radius * a.radius);
}

bool Collider2D::collisionCircleCircle(Collider2DCircle a, glm::vec2 aPos, Collider2DCircle b, glm::vec2 bPos) {
	return distance(bPos.x, bPos.y, aPos.x, aPos.y) < a.radius + b.radius;
}

bool Collider2D::collisionCircleLine(Collider2DCircle a, glm::vec2 aPos, Collider2DLine b, glm::vec2 bPos) {
	float hitDist = pointLineSegmentDistance(glm::vec2(bPos.x + b.sx, bPos.y + b.sy), glm::vec2(bPos.x + b.ex, bPos.y + b.ey), aPos);
	return hitDist < a.radius;
}

bool Collider2D::collisionCircleRotatedRectangle(Collider2DCircle a, glm::vec2 aPos, Collider2DRectangle b, glm::vec2 bPos, float brot) {
	return false;
	//grab the rotated, ordered points of this rectangle in space 
	//return circleIntersectsPolygon(a, aPos, b.points, 4);
}

bool Collider2D::collisionCirclePolygon(Collider2DCircle a, glm::vec2 aPos, Collider2DPolygon b, glm::vec2 bPos) {
	return circleIntersectsPolygon(a, aPos, &b.points[0], b.points.size());
}

bool Collider2D::collisionCircleRotatedPolygon(Collider2DCircle a, glm::vec2 aPos, Collider2DPolygon b, glm::vec2 bPos, float brot) {
	return circleIntersectsPolygon(a, aPos, &b.points[0], b.points.size());
}

bool Collider2D::circleIntersectsPolygon(Collider2DCircle a, glm::vec2 aPos, glm::vec2 points[], int numPoints) {
	for (int i = 0; i < numPoints - 1; ++i)
		if (pointLineSegmentDistance(points[i], points[i + 1], aPos) < a.radius)
			return true;
	return (pointLineSegmentDistance(points[0], points[numPoints - 1], aPos) < a.radius);
}

float Collider2D::pointLineSegmentDistance(glm::vec2 v, glm::vec2 w, glm::vec2 p) {
	// Return minimum distance between line segment vw and point p
	float l2 = (float)(pow(v.x - w.x, 2) + pow(v.y - w.y, 2));  // i.e. |w-v|^2 -  avoid a sqrt
	if (l2 == 0.0) return distance(p.x, p.y, v.x, v.y);   // v == w case
											// Consider the line extending the segment, parameterized as v + t (w - v).
											// We find projection of point p onto the line. 
											// It falls where t = [(p-v) . (w-v)] / |w-v|^2
											// We clamp t from [0,1] to handle points outside the segment vw.
	float t = std::max(0.f, std::min(1.f, glm::dot(p - v, w - v) / l2));
	glm::vec2 projection = v + t * (w - v);  // Projection falls on the segment
	return distance(p.x, p.y, projection.x, projection.y);
}

glm::vec2 Collider2D::closestPointOnLine(float lx1, float ly1, float lx2, float ly2, float x0, float y0) {
	float A1 = ly2 - ly1;
	float B1 = lx1 - lx2;
	double C1 = (ly2 - ly1) * lx1 + (lx1 - lx2) * ly1;
	double C2 = -B1 * x0 + A1 * y0;
	double det = A1 * A1 - -B1 * B1;
	float cx = 0;
	float cy = 0;
	if (det != 0) {
		cx = (float)((A1 * C1 - B1 * C2) / det);
		cy = (float)((A1 * C2 - -B1 * C1) / det);
	}
	else {
		cx = x0;
		cy = y0;
	}
	return glm::vec2(cx, cy);
}

void Collider2D::RotatePoint(glm::vec2& thePoint, glm::vec2& theOrigin, float theRotation) {
	float tempX = (float)(theOrigin.x + (thePoint.x - theOrigin.x) * cos(theRotation) - (thePoint.y - theOrigin.y) * sin(theRotation));
	thePoint.y = (float)(theOrigin.y + (thePoint.y - theOrigin.y) * cos(theRotation) + (thePoint.x - theOrigin.x) * sin(theRotation));
	thePoint.x = tempX;
}