#include "stdafx.h"
#pragma once
#include "GameObject.hpp"
#include "settings.hpp"
#include "mousePicking.hpp"

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

void checkDisplayObject() {
	if (mousePressedRight) {
		dFloat param;
		dVector posit, normal;
		std::pair<dVector, dVector> worldPoints = screenToWorld();
		NewtonBody* const body = MousePickByForce(world, worldPoints.first, worldPoints.second, param, posit, normal);
		if (body)
			displayObjectInfo((GameObject*)NewtonBodyGetUserData(body));
	}
}