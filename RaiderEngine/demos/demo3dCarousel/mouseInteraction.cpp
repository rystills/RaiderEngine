#include "stdafx.h"
#if (COMPILE_DEMO == DEMO_3D_CAROUSEL)
#include "mouseInteraction.hpp"
#include "GameObject.hpp"
#include "physics.hpp"
#include "input.hpp"

bool displayObjectInfo(GameObject* go) {
	displayString = go->getDisplayString();
	if (displayString.length() == 0)
		return false;
	mainCam->controllable = false;
	return true;
}

bool updateDisplayString() {
	if (mousePressedRight) {
		displayString.clear();
		mainCam->controllable = true;
		return true;
	}
	return false;
}

bool checkDisplayObject() {
	if (mousePressedRight) {
		PxRaycastBuffer hit = raycast(mainCam->Position, mainCam->Front, 1000);
		if (hit.hasBlock)
			return displayObjectInfo((GameObject*)hit.block.actor->userData);
	}
	return false;
}

void ReleaseHelpers() {
	if (gMouseJoint)
		gMouseJoint->release();
	gMouseJoint = NULL;
	if (gMouseSphere) {
		gScene->removeActor(*gMouseSphere);
		gMouseSphere->release();
		// set held object back to the default filter group
		PxShape* shape;
		hitBody->getShapes(&shape, 1);
		shape->setQueryFilterData(defaultFilterData);
	}
	gMouseSphere = NULL;
}

void updateHeldBody() {
	if (!sphereShape)
		sphereShape = gPhysics->createShape(PxSphereGeometry(.1f), *gMaterial, false, PxShapeFlag::eTRIGGER_SHAPE);
	if (mousePressedLeft) {
		PxRaycastBuffer hit = raycast(mainCam->Position, mainCam->Front, 1000);
		if (hit.hasBlock && hit.block.distance <= maxGrabRange && ((GameObject*)hit.block.actor->userData)->grabbable) {
			// grab object
			sphereDist = hit.block.distance;
			hitBody = hit.block.actor;
			// create collision point sphere actor
			gMouseSphere = gPhysics->createRigidDynamic(PxTransform(hit.block.position));
			gMouseSphere->attachShape(*sphereShape);
			gMouseSphere->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
			gScene->addActor(*gMouseSphere);

			//wake up hit object
			((PxRigidDynamic*)hitBody)->wakeUp();

			// move hit object to custom filter so it does not block raycasts while held
			hitShape = hit.block.shape;
			hit.block.shape->setQueryFilterData(noHitFilterData);

			// create joint between sphere and hit object
			// TODO: figure out localFrame calculation rather than forcing objects to be held by the center
			gMouseJoint = PxDistanceJointCreate(*gPhysics, gMouseSphere, PxTransform(PxVec3(0, 0, 0)), hitBody, PxTransform(PxVec3(0, 0, 0)));
		}
	}
	if (mouseHeldLeft && gMouseSphere) {
		// keep held object awake
		((PxRigidDynamic*)hitBody)->wakeUp();
		// reset held object angular velocity each step so that held objects can reorient themselves against collisions, but won't spin out of control
		((PxRigidDynamic*)hitBody)->setAngularVelocity(PxVec3(0, 0, 0), false);
		// reset held object linear velocity as well
		((PxRigidDynamic*)hitBody)->setLinearVelocity(PxVec3(0, 0, 0), false);
		// perform a sweep from the camera using the held body's shape and orientation to determine the precise destination
		PxQueryFilterData queryFilterData = PxQueryFilterData();
		queryFilterData.data = defaultFilterData;
		PxSweepBuffer hit;
		PxTransform castOrigin(PxVec3(mainCam->Position.x, mainCam->Position.y, mainCam->Position.z), hitBody->getGlobalPose().q);
		gScene->sweep(hitShape->getGeometry().convexMesh(), castOrigin, PxVec3(mainCam->Front.x,mainCam->Front.y,mainCam->Front.z),sphereDist,hit, PxHitFlag::eDEFAULT, queryFilterData);
		glm::vec3 newPos = mainCam->Position + mainCam->Front * (hit.hasBlock ? hit.block.distance : sphereDist);
		gMouseSphere->setGlobalPose(PxTransform(newPos.x, newPos.y, newPos.z));

	}
	else if (gMouseSphere)
		// let go of object
		ReleaseHelpers();
}
#endif