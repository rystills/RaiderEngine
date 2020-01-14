#pragma once
#include "Paddle.hpp"
#include "Ball.hpp"
inline int score = 0, level = 1;
inline bool paused = true;
#define levelMultiplier (1 + level * .2f)
void restartGame(bool clearScore = true);
void incrementLevel();
inline Paddle* paddle;
inline Ball* ball;