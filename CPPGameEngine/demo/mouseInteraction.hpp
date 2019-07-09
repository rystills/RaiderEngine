#include "../stdafx.h"
#pragma once
#include "../GameObject.hpp"
#include "../settings.hpp"

std::string displayString = "";

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
void checkDisplayObject(PxRaycastBuffer hit) {
	if (mousePressedRight && hit.hasBlock)
		displayObjectInfo((GameObject*)hit.block.actor->userData);
}