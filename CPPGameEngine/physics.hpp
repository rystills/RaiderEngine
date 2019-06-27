#include "stdafx.h"
#pragma once
NewtonWorld* world;
#define GRAVITY_STRENGTH 16.0f
// this file is responsible for initializing and maintaining the physics engine (currently Newton Dynamics)

/*
initialize newton physics
*/
void initPhysics() {
	world = NewtonCreate();
}

/*
cleanup the data allocated by newton physics
*/
void cleanupPhysics() {
	NewtonDestroyAllBodies(world);
	NewtonDestroy(world);
}