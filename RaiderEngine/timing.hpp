#pragma once
#include "stdafx.h"

// this file is responsible for maintaining timing information within the main game loop
inline double totalTime = 0.0;
inline float deltaTime = 0.0f;
inline double lastFrame = 0.0;
inline int framesThisSecond = 0;
inline double frameRenderTime = 0, avgRenderTime = 0, secondAvgRenderTime = 0;
inline double frameUpdateTime = 0, avgUpdateTime = 0, secondAvgUpdateTime = 0;
inline int fps = 0;
inline int lastTime = 0;

/*
update deltaTime based on the amount of time elapsed since the previous frame
*/
void updateTime();