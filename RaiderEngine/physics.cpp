#include "stdafx.h"
#include "physics.hpp"
#include "timing.hpp"
#include "graphics.hpp"
#include "terminalColors.hpp"

void initPhysics() {
	// create a foundation so we can use PhysX
	gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gDefaultAllocatorCallback, gDefaultErrorCallback);
	if (!gFoundation)
		ERRORCOLOR(std::cout << "PxCreateFoundation failed!" << std::endl);

	// create the top level physics object
	bool recordMemoryAllocations = true;
	gPvd = PxCreatePvd(*gFoundation);
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
	gPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);

	gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(), recordMemoryAllocations, gPvd);
	if (!gPhysics)
		ERRORCOLOR(std::cout << "PxCreatePhysics failed!" << std::endl);

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
	gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.4f);

	// cooking
	gCooking = PxCreateCooking(PX_PHYSICS_VERSION, *gFoundation, PxCookingParams(PxTolerancesScale()));
	if (!gCooking)
		ERRORCOLOR(std::cout << "PxCreateCooking failed!" << std::endl);

	// filter groups
	defaultFilterData.word0 = (1 << 0);
	noHitFilterData.word0 = (1 << 1);
	// debug visualization
	gScene->setVisualizationParameter(PxVisualizationParameter::eSCALE, 1.0f);
	gScene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES, 2.0f);
	gScene->setVisualizationParameter(PxVisualizationParameter::eACTOR_AXES, 1.0f);
}

void cleanupPhysics() {
	PX_RELEASE(gScene);
	PX_RELEASE(gDispatcher);
	PX_RELEASE(gPhysics);
	if (gPvd) {
		PxPvdTransport* transport = gPvd->getTransport();
		gPvd->release();
		gPvd = NULL;
		PX_RELEASE(transport);
	}
	PX_RELEASE(gFoundation);
}

void updatePhysics() {
	gScene->simulate(deltaTime);
	gScene->fetchResults(true);
}

void debugDrawPhysics() {
	const PxRenderBuffer& rb = gScene->getRenderBuffer();
	for (PxU32 i = 0; i < rb.getNbLines(); i++) {
		const PxDebugLine& line = rb.getLines()[i];
		// TODO: use correct state color, and draw PlayerController (same as before)
		queueDrawLine(glm::vec3(line.pos0.x, line.pos0.y, line.pos0.z), glm::vec3(line.pos1.x, line.pos1.y, line.pos1.z), stateColors[0]);
	}
}

PxRaycastBuffer raycast(glm::vec3 startPos, glm::vec3 dir, PxReal maxDistance, PxFilterData filterData) {
	PxVec3 origin(startPos.x, startPos.y, startPos.z);
	PxVec3 unitDir(dir.x, dir.y, dir.z);                // [in] Normalized ray direction
	PxRaycastBuffer hit;                 // [out] Raycast results
	PxQueryFilterData queryFilterData = PxQueryFilterData();
	queryFilterData.data = filterData;
	gScene->raycast(origin, unitDir, maxDistance, hit, PxHitFlag::eDEFAULT, queryFilterData);
	return hit;
}