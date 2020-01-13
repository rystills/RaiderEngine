#include "stdafx.h"
#include "FpsDisplay.hpp"
#include "timing.hpp"

void FpsDisplay::update() {
	text = "fps: " + std::to_string(fps);
}