#include "stdafx.h"
#if (COMPILE_DEMO == DEMO_3D_CAROUSEL)
#include "GameObject2D.hpp"
#include "Compass.hpp"
#include "settings.hpp"

void Compass::update() {
	GameObject2D::update();
	setRot(glm::radians(mainCam->Yaw));
}
#endif