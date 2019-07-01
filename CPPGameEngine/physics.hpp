#include "stdafx.h"
using namespace physx;

#pragma once
#define GRAVITY_STRENGTH 9.81f
static PxDefaultErrorCallback gDefaultErrorCallback;
static PxDefaultAllocator gDefaultAllocatorCallback;
#define PVD_HOST "127.0.0.1"
#define PX_RELEASE(x)	if(x)	{ x->release(); x = NULL;	}

PxFoundation* gFoundation = NULL;
PxPhysics* gPhysics = NULL;
PxDefaultCpuDispatcher* gDispatcher = NULL;
PxScene* gScene = NULL;
PxPvd* gPvd = NULL;
PxPvdSceneClient* pvdClient = NULL;
PxMaterial* gMaterial = NULL;

/*
initialize the physics engine
*/
void initPhysics() {
	// create a foundation so we can use PhysX
	gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gDefaultAllocatorCallback, gDefaultErrorCallback);
	if (!gFoundation)
		ERROR(std::cout << "PxCreateFoundation failed!" << std::endl);
	
	// create the top level physics object
	bool recordMemoryAllocations = true;
	gPvd = PxCreatePvd(*gFoundation);
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
	gPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);

	gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(), recordMemoryAllocations, gPvd);
	if (!gPhysics)
		ERROR(std::cout << "PxCreatePhysics failed!" << std::endl);

	// create physcs scene
	PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -GRAVITY_STRENGTH, 0.0f);
	gDispatcher = PxDefaultCpuDispatcherCreate(2);
	sceneDesc.cpuDispatcher = gDispatcher;
	sceneDesc.filterShader = PxDefaultSimulationFilterShader;
	gScene = gPhysics->createScene(sceneDesc);

	// create client
	pvdClient = gScene->getScenePvdClient();
	if (pvdClient) {
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
	}

	// create material
	gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.6f);
}

/*
cleanup the data allocated by the physics engine
*/
void cleanupPhysics() {
	PX_RELEASE(gScene);
	PX_RELEASE(gDispatcher);
	PX_RELEASE(gPhysics);
	if (gPvd) {
		PxPvdTransport* transport = gPvd->getTransport();
		gPvd->release();	gPvd = NULL;
		PX_RELEASE(transport);
	}
	PX_RELEASE(gFoundation);
}

/*
step the physics engine
*/
void updatePhysics(float deltaTime) {
	gScene->simulate(deltaTime);
	gScene->fetchResults(true);
}

//TODO: engine agnostic raycast call to replace mousePicking.hpp