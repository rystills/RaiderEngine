#include "../stdafx.h"
#pragma once
#include "../GameObject.hpp"
#include "../settings.hpp"

std::string displayString = "";
PxDistanceJoint* gMouseJoint = NULL;
PxRigidDynamic* gMouseSphere = NULL;
PxShape* sphereShape = NULL;
float sphereDist;

/*
display an information box detailing the specified object
@param go: the GameObject about which we wish to show information
*/
void displayObjectInfo(GameObject* go) {
	displayString = go->getDisplayString();
	player.camera.controllable = displayString.length() == 0;
}

/*
update the current display string, clearing it and reenabling camera control if the right mouse button is pressed
*/
void updateDisplayString() {
	if (mousePressedRight) {
		displayString.clear();
		player.camera.controllable = true;
	}
}

/*
if the user right clicked on an object, attempt to update the display string
*/
void checkDisplayObject() {
	if (mousePressedRight) {
		PxRaycastBuffer hit = raycast(player.camera.Position, player.camera.Front, 1000);
		if (hit.hasBlock)
			displayObjectInfo((GameObject*)hit.block.actor->userData);
	}
}

void ReleaseHelpers() {
	puts("D");
	if (gMouseJoint)
		gMouseJoint->release();
	gMouseJoint = NULL;
	if (gMouseSphere) {
		gScene->removeActor(*gMouseSphere);
		gMouseSphere->release();
	}
	gMouseSphere = NULL;

}

void updateHeldBody(float deltaTime) {
	if (!sphereShape)
		sphereShape = gPhysics->createShape(PxSphereGeometry(.1f), *gMaterial, false, PxShapeFlag::eTRIGGER_SHAPE);
	if (mousePressedLeft) {
		PxRaycastBuffer hit = raycast(player.camera.Position, player.camera.Front, 1000);
		puts("A");
		if (hit.hasBlock && ((GameObject*)hit.block.actor->userData)->grabbable) {
			// grab object
			puts("B");
			sphereDist = hit.block.distance;
			// create collision point sphere actor
			gMouseSphere = gPhysics->createRigidDynamic(PxTransform(hit.block.position));
			gMouseSphere->attachShape(*sphereShape);
			gMouseSphere->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC,true);
			gScene->addActor(*gMouseSphere);

			//wake up hit object
			((PxRigidDynamic*)hit.block.actor)->wakeUp();

			// create joint between sphere and hit object
			// TODO: localFrame1 calculation appears to be incorrect
			gMouseJoint = PxDistanceJointCreate(*gPhysics, gMouseSphere, PxTransform(PxVec3(0,0,0)), hit.block.actor, PxTransform(hit.block.actor->getGlobalPose().p - hit.block.position));
			puts("C");
		}
	}
	if (mouseHeldLeft && gMouseSphere) {
		// continue to hold object
		glm::vec3 newPos = player.camera.Position + player.camera.Front * sphereDist;
		// TODO: stop newPos short if an object passes in front of it
		gMouseSphere->setGlobalPose(PxTransform(newPos.x,newPos.y,newPos.z));
		
	}
	else if (gMouseSphere)
		// let go of object
		ReleaseHelpers();
}