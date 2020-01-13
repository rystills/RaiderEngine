#include "stdafx.h"
#pragma once
#include "GameObject.hpp"
#include "settings.hpp"
#include "input.hpp"

#define maxGrabRange 3
std::string displayString = "";
PxDistanceJoint* gMouseJoint = NULL;
PxRigidDynamic* gMouseSphere = NULL;
PxShape* sphereShape = NULL;
float sphereDist;
PxRigidActor* hitBody;

/*
display an information box detailing the specified object
@param go: the GameObject about which we wish to show information
*/
void displayObjectInfo(GameObject* go) {
	displayString = go->getDisplayString();
	mainCam->controllable = displayString.length() == 0;
}

/*
update the current display string, clearing it and reenabling camera control if the right mouse button is pressed
*/
void updateDisplayString() {
	if (mousePressedRight) {
		displayString.clear();
		mainCam->controllable = true;
	}
}

/*
if the user right clicked on an object, attempt to update the display string
*/
void checkDisplayObject() {
	if (mousePressedRight) {
		PxRaycastBuffer hit = raycast(mainCam->Position, mainCam->Front, 1000);
		if (hit.hasBlock)
			displayObjectInfo((GameObject*)hit.block.actor->userData);
	}
}

/*
clear the mouse joint and sphere upon releasing the held object
*/
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

/*
update the held body, allowing the user to grab, hold, or let go of any grabbable GameObject
*/
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
			gMouseSphere->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC,true);
			gScene->addActor(*gMouseSphere);

			//wake up hit object
			((PxRigidDynamic*)hitBody)->wakeUp();
			
			// move hit object to custom filter so it does not block raycasts while held
			hit.block.shape->setQueryFilterData(noHitFilterData);

			// create joint between sphere and hit object
			// TODO: figure out localFrame calculation rather than forcing objects to be held by the center
			gMouseJoint = PxDistanceJointCreate(*gPhysics, gMouseSphere, PxTransform(PxVec3(0,0,0)), hitBody, PxTransform(PxVec3(0,0,0)));
		}
	}
	if (mouseHeldLeft && gMouseSphere) {
		// keep held object awake
		((PxRigidDynamic*)hitBody)->wakeUp();
		// reset held object angular velocity each step so that held objects can reorient themselves against collisions, but won't spin out of control
		((PxRigidDynamic*)hitBody)->setAngularVelocity(PxVec3(0, 0, 0), false);
		// reset held object linear velocity as well
		((PxRigidDynamic*)hitBody)->setLinearVelocity(PxVec3(0, 0, 0), false);
		// continue to hold object
		PxRaycastBuffer hit = raycast(mainCam->Position, mainCam->Front, sphereDist);
		// move held object sphere in front of obstacles to prevent it from clipping through walls / into other objects
		glm::vec3 newPos = mainCam->Position + mainCam->Front * (hit.hasBlock ? hit.block.distance : sphereDist);
		gMouseSphere->setGlobalPose(PxTransform(newPos.x,newPos.y,newPos.z));
		
	}
	else if (gMouseSphere)
		// let go of object
		ReleaseHelpers();
}