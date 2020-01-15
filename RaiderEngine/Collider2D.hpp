#pragma once
#include "stdafx.h"
#include <algorithm>
#include <math.h>
extern class Collider2DCircle;
extern class Collider2DLine;
extern class Collider2DPolygon;
extern class Collider2DRectangle;

// class representing a collider used for collision checking between objects
// because the player and enemies strictly use circle colliders, collision definitions are mostly limited to those involving a circle
class Collider2D {
public:
	/*
	Collider constructor: define the shape and collider type
	*/
	Collider2D() { }

	/*
	check whether two colliders are overlapping (true) or not (false)
	@param myPos: the position of this collider
	@param other: the collider with which to check for a collision
	@param otherPos: the position of the other collider
	@param otherRot: the rotation of the other collider
	@returns: whether the two rectangles are overlapping (true) or not (false)
	*/
	virtual bool collision(glm::vec2 myPos, float myRot, Collider2D* other, glm::vec2 otherPos, float otherRot);

	static bool linesIntersect(glm::vec2 pt1, glm::vec2 pt2, glm::vec2 pt3, glm::vec2 pt4);

	static bool pointInPoly(glm::vec2 pt, glm::vec2 polyPts[], int numPts);

	virtual void debugDraw(glm::vec2 pos, float rot) {};

	/*
	check whether two rectangles are overlapping (true) or not (false)
	@param a: the first rectangle
	@param aPos: the position of the first rectangle
	@param b: the second rectangle
	@param bPos: the position of the second rectangle
	@returns: whether the two rectangles are overlapping (true) or not (false)
	*/
	static bool collisionRectangleRectangle(Collider2DRectangle a, glm::vec2 aPos, Collider2DRectangle b, glm::vec2 bPos);

	static bool collisionRectangleRotatedRectangle(Collider2DRectangle a, glm::vec2 aPos, float arot, Collider2DRectangle b, glm::vec2 bPos, float brot);

	static bool collisionRectanglePolygon(Collider2DRectangle a, glm::vec2 aPos, float arot, Collider2DPolygon b, glm::vec2 bPos, float brot);

	/*
	check whether a circle and a rectangle are overlapping (true) or not (false)
	@param a: the circle
	@param aPos: the position of the circle
	@param b: the rectangle
	@param bPos: the position of the rectangle
	@returns: whether a circle and a rectangle are overlapping (true) or not (false)
	*/
	static bool collisionCircleRectangle(Collider2DCircle a, glm::vec2 aPos, Collider2DRectangle b, glm::vec2 bPos);

	bool collisionLineRectangle(Collider2DLine a, glm::vec2 aPos, float aRot, Collider2DRectangle b, glm::vec2 bPos, float bRot);

	bool collisionLinePolygon(Collider2DLine a, glm::vec2 aPos, float arot, Collider2DPolygon b, glm::vec2 bPos, float brot);

	/*
	check whether a circle and a circle are overlapping (true) or not (false)
	@param a: the first circle
	@param aPos: the position of the first circle
	@param b: the second circle
	@param bPos: the position of the second rectangle
	@returns: whether a circle and a circle are overlapping (true) or not (false)
	*/
	static bool collisionCircleCircle(Collider2DCircle a, glm::vec2 aPos, Collider2DCircle b, glm::vec2 bPos);

	static bool collisionLineLine(Collider2DLine a, glm::vec2 aPos, float arot, Collider2DLine b, glm::vec2 bPos, float brot);

	static bool collisionCircleRotatedLine(Collider2DCircle a, glm::vec2 aPos, Collider2DLine b, glm::vec2 bPos, float brot);

	/*
	check whether a circle and a line are overlapping (true) or not (false)
	@param a: the circle
	@param aPos: the position of the circle
	@param b: the line
	@param bPos: the position of the line
	@returns: whether a circle and a line are overlapping (true) or not (false)
	*/
	static bool collisionCircleLine(Collider2DCircle a, glm::vec2 aPos, Collider2DLine b, glm::vec2 bPos);

	/*
	check whether a circle and a rotated Rectangle are overlapping (true) or not (false)
	@param a: the circle
	@param aPos: the position of the circle
	@param b: the RotatedRectangle
	@param bPos: the position of the rotated rectangle
	@param brot: the rotation (in degrees) of the rectangle
	@returns: whether a circle and a line are overlapping (true) or not (false)
	*/
	static bool collisionCircleRotatedRectangle(Collider2DCircle a, glm::vec2 aPos, Collider2DRectangle b, glm::vec2 bPos, float brot);

	/*
	check whether a circle and a Polygon are overlapping (true) or not (false)
	@param a: the circle
	@param aPos: the position of the circle
	@param b: the Polygon
	@param bPos: the position of the polygon
	@returns: whether a circle and a Polygon are overlapping (true) or not (false)
	*/
	static bool collisionCirclePolygon(Collider2DCircle a, glm::vec2 aPos, Collider2DPolygon b, glm::vec2 bPos);

	/*
	check whether a circle and a rotated Polygon are overlapping (true) or not (false)
	@param a: the circle
	@param aPos: the position of the circle
	@param b: the RotatedPolygon
	@param bPos: the position of the rotated polygon
	@param brot: the rotation (in degrees) of the polygon
	@returns: whether a circle and a RotatedPolygon are overlapping (true) or not (false)
	*/
	static bool collisionCircleRotatedPolygon(Collider2DCircle a, glm::vec2 aPos, Collider2DPolygon b, glm::vec2 bPos, float brot);

	static bool collisionPolygonPolygon(Collider2DPolygon a, glm::vec2 aPos, float arot, Collider2DPolygon b, glm::vec2 bPos, float brot);

	/*
	determine whether or not a circle intersects a polygon (specified by an ordered point list)
	@param a: the circle
	@param aPos: the position of the circle rectangle
	@param points: the ordered point list representing the polygon
	@returns: whether the circle intersects any of the lines comprising the polygon (true) or not (false)
	*/
	static bool circleIntersectsPolygon(Collider2DCircle a, glm::vec2 aPos, glm::vec2 points[], int numPoints);

	/*
	find the shortest distance between a line segment and another point - code from https://stackoverflow.com/questions/849211/shortest-distance-between-a-point-and-a-line-segment
	@param v: the first point on the line
	@param w: the second point on the line
	@param p: the other point used for comparison
	@returns: the closest distance on the specified line segment from the specified point
	*/
	static float pointLineSegmentDistance(glm::vec2 v, glm::vec2 w, glm::vec2 p);

	/*
	find the closest point on a line from another point - code from http://ericleong.me/research/circle-line/
	@param lx1: the x1 coordinate of the line
	@param ly1: the y1 coordinate of the line
	@param lx2: the x2 coordinate of the line
	@param ly2: the y2 coordinate of the line
	@param x0: the x coordinate of the point
	@param y0: the y coordinate of the point
	@returns: a glm::vec2 containing the closest point on the specified line from the specified point
	*/
	static glm::vec2 closestPointOnLine(float lx1, float ly1, float lx2, float ly2, float x0, float y0);

	/*
	find the distance between two points
	@param x1: the x coordinate of the first point
	@param y1: the y coordinate of the first point
	@param x2: the x coordinate of the second point
	@param y2: the y coordinate of the second point
	@returns: the distance between two points
	*/
	static float distance(float x1, float y1, float x2, float y2) {
		return (float)sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2));
	}

	/*
	Rotate a point from a given location and adjust using the Origin we are
	rotating around -- code from http://www.xnadevelopment.com/tutorials/rotatedrectanglecollisions/rotatedrectanglecollisions.shtml
	@param thePoint: the point to rotate
	@param theOrigin: the origin point around which to rotate our point
	@param theRotation: the amount (in radians) by which to rotate the point
	@returns: the new location of our point after rotating around the specified origin
	*/
	static void RotatePoint(glm::vec2& thePoint, glm::vec2& theOrigin, float theRotation);
};