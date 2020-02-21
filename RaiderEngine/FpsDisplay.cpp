#include "stdafx.h"
#include "FpsDisplay.hpp"
#include "timing.hpp"

void FpsDisplay::update() {
	text = "fps: " + std::to_string(fps) + " | update time: " + std::to_string(secondAvgUpdateTime) + " | render time: " + std::to_string(secondAvgRenderTime);
}