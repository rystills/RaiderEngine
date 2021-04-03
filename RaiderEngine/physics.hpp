using namespace physx;
#pragma once
#include "stdafx.h"
#define GRAVITY_STRENGTH 9.81f
inline static PxDefaultErrorCallback gDefaultErrorCallback;
inline static PxDefaultAllocator gDefaultAllocatorCallback;
#define PVD_HOST "127.0.0.1"
#define PX_RELEASE(x)	if(x)	{ x->release(); x = NULL;	}

inline PxFoundation* gFoundation = NULL;
inline PxPhysics* gPhysics = NULL;
inline PxDefaultCpuDispatcher* gDispatcher = NULL;
inline PxScene* gScene = NULL;
inline PxPvd* gPvd = NULL;
inline PxPvdSceneClient* pvdClient = NULL;
inline PxMaterial* gMaterial = NULL;
inline PxCooking* gCooking = NULL;
inline PxFilterData defaultFilterData;
inline PxFilterData triggerFilterData;
inline PxFilterData noHitFilterData;

/*
initialize the physics engine
*/
void initPhysics();

/*
cleanup the data allocated by the physics engine
*/
void cleanupPhysics();

/*
step the physics engine
*/
void updatePhysics();

void queueDrawLine(const glm::vec3& from, const glm::vec3& to, const glm::vec3& color);
void drawLines();
/*
draw the physics wireframe
*/
void debugDrawPhysics();

/*
perform a raycast from the specified position in the specified direction, returning the closest hit
@param startPos: the position in world space from which to start the raycast
@param dir: the direction of the raycast
@param maxDistance: the length of the raycast
@param filterData: the shape filter group to check against
@param hitFlag: the hit flag to use
@returns: a PxRaycastBuffer containing the hit data
*/
PxRaycastBuffer raycast(glm::vec3 startPos, glm::vec3 dir, PxReal maxDistance, PxFilterData filterData = defaultFilterData, PxHitFlag::Enum hitFlag = PxHitFlag::eDEFAULT);

class GEventCallback : public PxSimulationEventCallback {
public:
	GEventCallback() { };
	virtual void onTrigger(PxTriggerPair* pairs, PxU32 count) override;

	virtual void							onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs) {};
	virtual void							onConstraintBreak(PxConstraintInfo*, PxU32) {}
	virtual void							onWake(PxActor**, PxU32) {}
	virtual void							onSleep(PxActor**, PxU32) {}
	virtual void							onAdvance(const PxRigidBody* const*, const PxTransform*, const PxU32) {}
} inline gEventCallback;