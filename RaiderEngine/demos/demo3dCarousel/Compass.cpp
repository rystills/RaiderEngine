#include "stdafx.h"
#include "GameObject2D.hpp"
#include "Compass.hpp"
#include "settings.hpp"

void Compass::update() {
	GameObject2D::update();
	rotation = glm::radians(mainCam->Yaw);
}