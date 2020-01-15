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

bool Collider2D::linesIntersect(glm::vec2 pt1, glm::vec2 pt2, glm::vec2 pt3, glm::vec2 pt4) {
	#define ccw(p1,p2,p3) (p3.y - p1.y) * (p2.x - p1.x) > (p2.y - p1.y) * (p3.x - p1.x)
	return ccw(pt1, pt3, pt4) != ccw(pt2, pt3, pt4) && ccw(pt1, pt2, pt3) != ccw(pt1, pt2, pt4);
}

bool Collider2D::pointInPoly(glm::vec2 pt, glm::vec2 polyPts[], int numPts) {
	bool collision = false;
	for (int i = 0; i < numPts; ++i) {
		glm::vec2 vc(polyPts[i]), vn(polyPts[i == numPts-1 ? 0 : i + 1]);
		if (((vc.y >= pt.y && vn.y < pt.y) || (vc.y < pt.y && vn.y >= pt.y)) && (pt.x < (vn.x - vc.x) * (pt.y - vc.y) / (vn.y - vc.y) + vc.x))
			collision = !collision;
	}
	return collision;
}

bool Collider2D::collisionRectangleRectangle(Collider2DRectangle a, glm::vec2 aPos, Collider2DRectangle b, glm::vec2 bPos) {
	return !(
		(aPos.x - a.hwidth > bPos.x + b.hwidth || bPos.x - b.hwidth > aPos.x + a.hwidth) ||
		(aPos.y - a.hheight > bPos.y + b.hheight || bPos.y - b.hheight > aPos.y + a.hheight)
		);
}

bool Collider2D::collisionRectangleRotatedRectangle(Collider2DRectangle a, glm::vec2 aPos, float arot, Collider2DRectangle b, glm::vec2 bPos, float brot) {
	glm::vec2 apoints[4];
	a.getRotatedCornerPoints(apoints, aPos, arot);
	glm::vec2 bpoints[4];
	b.getRotatedCornerPoints(bpoints, bPos, brot);
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j) {
			if (linesIntersect(apoints[i], apoints[i == 3 ? 0 : i + 1], bpoints[j], bpoints[j == 3 ? 0 : j + 1]))
				return true;
		}
		
	// no intersections; check if either rectangle fully contains the other
	return pointInPoly(apoints[0], bpoints, 4) || pointInPoly(bpoints[0], apoints, 4);
}

bool Collider2D::collisionRectanglePolygon(Collider2DRectangle a, glm::vec2 aPos, float arot, Collider2DPolygon b, glm::vec2 bPos, float brot) {
	glm::vec2 apoints[4];
	a.getRotatedCornerPoints(apoints, aPos, arot);
	std::vector<glm::vec2> bpoints(b.points.size(), glm::vec2(0));
	b.getRotatedPoints(&bpoints[0], bPos, brot);
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < bpoints.size(); ++j) {
			if (linesIntersect(apoints[i], apoints[i == 3 ? 0 : i + 1], bpoints[j], bpoints[j == bpoints.size()-1 ? 0 : j + 1]))
				return true;
		}

	// no intersections; check if either rectangle fully contains the other
	return pointInPoly(apoints[0], &bpoints[0], bpoints.size()) || pointInPoly(bpoints[0], apoints, 4);
}

bool Collider2D::collisionCircleRectangle(Collider2DCircle a, glm::vec2 aPos, Collider2DRectangle b, glm::vec2 bPos) {
	float deltaX = aPos.x - std::max(bPos.x - b.hwidth, std::min(aPos.x, bPos.x + b.hwidth));
	float deltaY = aPos.y - std::max(bPos.y - b.hheight, std::min(aPos.y, bPos.y + b.hheight));
	return (deltaX * deltaX + deltaY * deltaY) < (a.radius * a.radius);
}

bool Collider2D::collisionLineRectangle(Collider2DLine a, glm::vec2 aPos, float arot, Collider2DRectangle b, glm::vec2 bPos, float brot) {
	glm::vec2 pt1(aPos.x + a.sx, aPos.y + a.sy), pt2(aPos.x + a.ex, aPos.y + a.ey);
	RotatePoint(pt1, aPos, arot);
	RotatePoint(pt2, aPos, arot);
	glm::vec2 points[4];
	b.getRotatedCornerPoints(points, bPos, brot);
	// check for any intersections
	for (int i = 0; i < 4; ++i)
		if (linesIntersect(pt1,pt2,points[i],points[i==3?0:i+1]))
			return true;
	// no intersections; check if the line segment is fully contained within the rectangle
	return pointInPoly(pt1, points, 4);
}

bool Collider2D::collisionLinePolygon(Collider2DLine a, glm::vec2 aPos, float arot, Collider2DPolygon b, glm::vec2 bPos, float brot) {
	glm::vec2 pt1(aPos.x + a.sx, aPos.y + a.sy), pt2(aPos.x + a.ex, aPos.y + a.ey);
	RotatePoint(pt1, aPos, arot);
	RotatePoint(pt2, aPos, arot);
	std::vector<glm::vec2> points(b.points.size(), glm::vec2(0));
	b.getRotatedPoints(&points[0], bPos, brot);
	// check for any intersections
	for (int i = 0; i < points.size(); ++i)
		if (linesIntersect(pt1,pt2,points[i],points[i==points.size()-1?0:i+1]))
			return true;
	// no intersections; check if the line segment is fully contained within the rectangle
	return pointInPoly(pt1, &points[0], points.size());
}

bool Collider2D::collisionCircleCircle(Collider2DCircle a, glm::vec2 aPos, Collider2DCircle b, glm::vec2 bPos) {
	return distance(bPos.x, bPos.y, aPos.x, aPos.y) < a.radius + b.radius;
}

bool Collider2D::collisionCircleLine(Collider2DCircle a, glm::vec2 aPos, Collider2DLine b, glm::vec2 bPos) {
	float hitDist = pointLineSegmentDistance(glm::vec2(bPos.x + b.sx, bPos.y + b.sy), glm::vec2(bPos.x + b.ex, bPos.y + b.ey), aPos);
	return hitDist < a.radius;
}

bool Collider2D::collisionCircleRotatedLine(Collider2DCircle a, glm::vec2 aPos, Collider2DLine b, glm::vec2 bPos, float brot) {
	glm::vec2 points[2];
	b.getRotatedPoints(points, bPos, brot);
	float hitDist = pointLineSegmentDistance(points[0],points[1], aPos);
	return hitDist < a.radius;
}

bool Collider2D::collisionLineLine(Collider2DLine a, glm::vec2 aPos, float arot, Collider2DLine b, glm::vec2 bPos, float brot) {
	// note: this does not account for colinearity
	glm::vec2 pt1(aPos.x + a.sx, aPos.y + a.sy), pt3(bPos.x + b.sx, bPos.y + b.sy);
	glm::vec2 pt2(aPos.x + a.ex, aPos.y + a.ey), pt4(bPos.x + b.ex, bPos.y + b.ey);
	RotatePoint(pt1, aPos, arot);
	RotatePoint(pt2, aPos, arot);
	RotatePoint(pt3, bPos, brot);
	RotatePoint(pt4, bPos, brot);
	return linesIntersect(pt1,pt2,pt3,pt4);
}

bool Collider2D::collisionCircleRotatedRectangle(Collider2DCircle a, glm::vec2 aPos, Collider2DRectangle b, glm::vec2 bPos, float brot) {
	//grab the rotated, ordered points of this rectangle in space
	glm::vec2 points[4];
	b.getRotatedCornerPoints(points, bPos, brot);
	return circleIntersectsPolygon(a, aPos, points, 4);
}

bool Collider2D::collisionCirclePolygon(Collider2DCircle a, glm::vec2 aPos, Collider2DPolygon b, glm::vec2 bPos) {
	// rather than applying bPos to all points in b, simply offset our position by -bPos
	return circleIntersectsPolygon(a, aPos-bPos, &b.points[0], b.points.size());
}

bool Collider2D::collisionCircleRotatedPolygon(Collider2DCircle a, glm::vec2 aPos, Collider2DPolygon b, glm::vec2 bPos, float brot) {
	std::vector<glm::vec2> points(b.points.size(),glm::vec2(0));
	b.getRotatedPoints(&points[0], bPos, brot);
	return circleIntersectsPolygon(a, aPos, &points[0], b.points.size());
}

bool Collider2D::collisionPolygonPolygon(Collider2DPolygon a, glm::vec2 aPos, float arot, Collider2DPolygon b, glm::vec2 bPos, float brot) {
	// TODO: consider forcing convex polygons and switching to separating axis theorem to speed up polygon collision checking
	std::vector<glm::vec2> apoints(a.points.size(), glm::vec2(0));
	a.getRotatedPoints(&apoints[0], aPos, arot);
	std::vector<glm::vec2> bpoints(b.points.size(), glm::vec2(0));
	b.getRotatedPoints(&bpoints[0], bPos, brot);
	for (int i = 0; i < apoints.size(); ++i)
		for (int j = 0; j < bpoints.size(); ++j) {
			if (linesIntersect(apoints[i], apoints[i == apoints.size() - 1 ? 0 : i + 1], bpoints[j], bpoints[j == bpoints.size() - 1 ? 0 : j + 1]))
				return true;
		}

	// no intersections; check if either rectangle fully contains the other
	return pointInPoly(apoints[0], &bpoints[0], bpoints.size()) || pointInPoly(bpoints[0], &apoints[0], apoints.size());
}

bool Collider2D::circleIntersectsPolygon(Collider2DCircle a, glm::vec2 aPos, glm::vec2 points[], int numPoints) {
	for (int i = 0; i < numPoints - 1; ++i)
		if (pointLineSegmentDistance(points[i], points[i + 1], aPos) < a.radius)
			return true;
	return (pointLineSegmentDistance(points[0], points[numPoints - 1], aPos) < a.radius) || pointInPoly(aPos, points, numPoints);
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