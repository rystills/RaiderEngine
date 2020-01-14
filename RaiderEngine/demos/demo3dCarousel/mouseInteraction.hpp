#include "stdafx.h"
#pragma once
#include "GameObject.hpp"
#include "settings.hpp"
#include "input.hpp"

#define maxGrabRange 3
inline std::string displayString = "";
inline PxDistanceJoint* gMouseJoint = NULL;
inline PxRigidDynamic* gMouseSphere = NULL;
inline PxShape* sphereShape = NULL;
inline float sphereDist;
inline PxRigidActor* hitBody;

/*
display an information box detailing the specified object
@param go: the GameObject about which we wish to show information
*/
void displayObjectInfo(GameObject* go);

/*
update the current display string, clearing it and reenabling camera control if the right mouse button is pressed
*/
void updateDisplayString();

/*
if the user right clicked on an object, attempt to update the display string
*/
void checkDisplayObject();

/*
clear the mouse joint and sphere upon releasing the held object
*/
void ReleaseHelpers();
/*
update the held body, allowing the user to grab, hold, or let go of any grabbable GameObject
*/
void updateHeldBody();