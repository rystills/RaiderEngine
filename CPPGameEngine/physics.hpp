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
PxCooking* gCooking = NULL;
PxFilterData defaultFilterData;

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

	// cooking
	gCooking = PxCreateCooking(PX_PHYSICS_VERSION, *gFoundation, PxCookingParams(PxTolerancesScale()));
	if (!gCooking)
		ERROR(std::cout << "PxCreateCooking failed!" << std::endl);

	// default filter group
	defaultFilterData.word0 = (1 << 0);

	// debug visualization
	gScene->setVisualizationParameter(PxVisualizationParameter::eSCALE, 1.0f);
	gScene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES, 2.0f);
	gScene->setVisualizationParameter(PxVisualizationParameter::eACTOR_AXES, 1.0f);
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

void queueDrawLine(const glm::vec3& from, const glm::vec3& to, const glm::vec3& color);
void drawLines();
extern glm::vec3 stateColors[3];
/*
draw the physics wireframe
*/
void debugDrawPhysics() {
	const PxRenderBuffer& rb = gScene->getRenderBuffer();
	for (PxU32 i = 0; i < rb.getNbLines(); i++) {
		const PxDebugLine& line = rb.getLines()[i];
		// TODO: use correct state color, and draw PlayerController (same as before)
		queueDrawLine(glm::vec3(line.pos0.x, line.pos0.y, line.pos0.z), glm::vec3(line.pos1.x, line.pos1.y, line.pos1.z), stateColors[2]);
	}
	drawLines();
}

/*
perform a raycast from the specified position in the specified direction, returning the closest hit
@param startPos: the position in world space from which to start the raycast
@param dir: the direction of the raycast
@param maxDistance: the length of the raycast
@param filterData: the shape filter group to check against
@returns: a PxRaycastBuffer containing the hit data
*/
PxRaycastBuffer raycast(glm::vec3 startPos, glm::vec3 dir, PxReal maxDistance, PxFilterData filterData = defaultFilterData) {
	PxVec3 origin(startPos.x, startPos.y, startPos.z);
	PxVec3 unitDir(dir.x, dir.y, dir.z);                // [in] Normalized ray direction
	PxRaycastBuffer hit;                 // [out] Raycast results
	PxQueryFilterData queryFilterData = PxQueryFilterData();
	queryFilterData.data = filterData;
	gScene->raycast(origin, unitDir, maxDistance, hit, PxHitFlag::eDEFAULT, queryFilterData);
	return hit;
}