#include "stdafx.h"
#pragma once
//NewtonWorld* world;
#define GRAVITY_STRENGTH 16.0f
// this file is responsible for initializing and maintaining the physics engine (currently Newton Dynamics)

/*
initialize the physics engine
*/
void initPhysics() {
	
}

/*
cleanup the data allocated by the physics engine
*/
void cleanupPhysics() {
	
}

/*
step the physics engine
*/
void updatePhysics() {
	//NewtonUpdate(world, deltaTime);
}

//TODO: engine agnostic raycast call to replace mousePicking.hpp